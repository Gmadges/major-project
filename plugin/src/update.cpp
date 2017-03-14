#include "update.h"
#include "hackPrint.h"
#include "messaging.h"

#include "genericMeshMessage.h"

#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MPlug.h"
#include "maya/MArgDatabase.h"
#include "maya/MPlugArray.h"

Update::Update()
	:
	pMessenger(new Messaging("localhost",8080))
{
}

Update::~Update()
{
}

void* Update::creator()
{
	return new Update;
}

MSyntax Update::newSyntax()
{
	MSyntax syn;

	syn.addFlag("-a", "-address", MSyntax::kString);
	syn.addFlag("-p", "-port", MSyntax::kUnsigned);

	return syn;
}

MStatus	Update::doIt(const MArgList& args)
{
	MStatus status = MStatus::kSuccess;

	// reset socket
	MString addr;
	int port;
	status = getArgs(args, addr, port);
	if (status != MStatus::kSuccess)
	{
		HackPrint::print("no input values specified");
		return status;
	}

	pMessenger->resetSocket(std::string(addr.asChar()), port);

	// ask the server for any update
	GenericMesh data;
	
	// if false then we couldnt connect to server
	if (!pMessenger->requestData(data)) return MStatus::kFailure;
	
	// is there actually anything?
	if (data.getNodes().empty())
	{
		HackPrint::print("Nothing to update");
		return status;
	}

	// check if mesh exists
	bool meshexists = doesItExist(MString(data.getMeshName().c_str()));

	if (!meshexists)
	{
		HackPrint::print("Mesh doesnt exist");

		// create mesh
		// does nothing atm
		createMesh(data);
	}

	// get nodes
	auto nodeList = data.getNodes();

	for (GenericNode itr : nodeList)
	{
		// check if node exists
		MString nodeName = itr.getNodeName().c_str();
		bool nodeExists = doesItExist(nodeName);

		if (!nodeExists)
		{
			HackPrint::print("Creating Node: " + nodeName);
			// create set and wire
			createNode(itr);
			setNodeValues(itr);
			setConnections(data, itr);
			continue;
		}
		HackPrint::print("No need to create " + nodeName);
		// set values
		// no need to worry about re-wireing anything
		// just change the values
		setNodeValues(itr);
	}
	return status;
}

MStatus Update::setNodeValues(GenericNode & data)
{
	MSelectionList sList;
	sList.add(data.getNodeName().c_str());
	MObject node;
	if (sList.getDependNode(0, node) != MStatus::kSuccess) return MStatus::kFailure;
	// rename and set correct details
	MFnDependencyNode depNode(node);

	depNode.setName(MString(data.getNodeName().c_str()));

	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	auto dataAttribs = data.getAttribs();
	MStatus status;

	for (auto atr : dataAttribs)
	{
		MPlug plug = depNode.findPlug(atr.first.c_str(), &status);

		if (status == MStatus::kSuccess)
		{
			switch (atr.second.type)
			{
				case msgpack::type::BOOLEAN:
				{
					plug.setBool(atr.second.via.boolean);
					break;
				}
				case msgpack::type::FLOAT:
				{
					plug.setFloat(atr.second.via.f64);
					break;
				}
				case msgpack::type::STR:
				{
					std::string val;
					atr.second.convert(val);
					plug.setString(MString(val.c_str()));
					break;
				}
				case msgpack::type::NEGATIVE_INTEGER:
				{
					int val;
					atr.second.convert(val);
					plug.setInt(val);
					break;
				}
				case msgpack::type::POSITIVE_INTEGER:
				{
					plug.setInt64(atr.second.via.i64); 
					break;
				}
				case msgpack::type::ARRAY:
				{
					// at the moment the only thing that will use this will be tweaks
					
					// if plug == tweak

					// then look at the array and apply

					// need to store or indice and x/y/z and val

					break;
				}
			}			
		}
	}
	return MStatus::kSuccess;
}

MStatus Update::getArgs(const MArgList& args, MString& address, int& port)
{
	MStatus status = MStatus::kSuccess;
	MArgDatabase parser(syntax(), args, &status);

	if (status != MS::kSuccess) return status;

	// get the command line arguments that were specified
	if (parser.isFlagSet("-p"))
	{
		parser.getFlagArgument("-p", 0, port);
	}
	else
	{
		status = MStatus::kFailure;
	}

	if (parser.isFlagSet("-a"))
	{
		parser.getFlagArgument("-a", 0, address);
	}
	else
	{
		status = MStatus::kFailure;
	}

	return status;
}

MStatus Update::createMesh(GenericMesh& _mesh)
{
	MStatus status;

	// create a mesh
	switch (_mesh.getMeshType())
	{
		case MeshType::CUBE:
		{
			MString cmd;
			cmd += "polyCube";

			HackPrint::print(cmd);
			MStringArray result;
			status = MGlobal::executeCommand(cmd, result);

			if (status != MStatus::kSuccess) return status;

			// rename the nodes
			MSelectionList sList;
			sList.add(result[0]);
			MObject node;
			if (sList.getDependNode(0, node) != MStatus::kSuccess) return MStatus::kFailure;

			// rename and set correct details
			MFnDependencyNode depNode(node);

			renameNodes(depNode, _mesh);
			
			return status;
		}
	}

	return MStatus::kFailure;
}

void Update::renameNodes(MFnDependencyNode & node, GenericMesh& mesh)
{
	// rename
	// This wont work for multiple 
	for (auto it : mesh.getNodes())
	{
		MString type(it.getNodeType().c_str());
		if (node.typeName() == type)
		{
			node.setName(MString(it.getNodeName().c_str()));
		}
	}	

	// If the inputPolymesh is connected, we have history
	MStatus status;
	MPlug inMeshPlug;
	inMeshPlug = node.findPlug("inputPolymesh", &status);

	// if it doesnt have that plug try this one
	if (status != MStatus::kSuccess)
	{
		inMeshPlug = node.findPlug("inMesh");
	}

	if (inMeshPlug.isConnected())
	{
		MPlugArray tempPlugArray;
		inMeshPlug.connectedTo(tempPlugArray, true, false);
		// Only one connection should exist on meshNodeShape.inMesh!
		MPlug upstreamNodeSrcPlug = tempPlugArray[0];
		MFnDependencyNode upstreamNode(upstreamNodeSrcPlug.node());

		renameNodes(upstreamNode, mesh);
	}
}

MStatus Update::createNode(GenericNode& _node)
{
	MStatus status;

	// create a node of same type?
	MString cmd;
	cmd += "createNode \"";
	cmd += _node.getNodeType().c_str();
	cmd += "\"";

	// node name
	cmd += " -n \"";
	cmd += _node.getNodeName().c_str();
	cmd += "\"";

	status = MGlobal::executeCommand(cmd);
	return status;
}

MStatus Update::setConnections(GenericMesh& _mesh, GenericNode& _node)
{
	// if its not a mesh we'll have to wire it in
	if (_node.getNodeType().compare("polySplitRing") == 0 ||
		_node.getNodeType().compare("polyTweak") == 0 )
	{
		// get mesh and set it to be the one we're effecting
		MSelectionList selList;
		selList.add(MString(_mesh.getMeshName().c_str()));
		MDagPath dagpath;
		selList.getDagPath(0, dagpath);
		dagpath.extendToShape();
		setMeshNode(dagpath);

		// get node and do the connections
		MSelectionList sList;
		sList.add(_node.getNodeName().c_str());
		MObject node;
		if (sList.getDependNode(0, node) != MStatus::kSuccess) return MStatus::kFailure;

		//// and add it to the DAG
		doModifyPoly(node);

		// this is if we require extra connections
		if (_node.getNodeType().compare("polySplitRing") == 0)
		{
			MString connectCmd;
			connectCmd += "connectAttr ";
			connectCmd += _mesh.getMeshName().c_str();
			connectCmd += ".worldMatrix[0] ";
			connectCmd += _node.getNodeName().c_str();
			connectCmd += ".manipMatrix;";
			MGlobal::executeCommand(connectCmd);
		}
	}

	return MStatus::kSuccess;
}

bool Update::doesItExist(MString& name)
{
	MString objExistsCmd;
	objExistsCmd += "objExists \"";
	objExistsCmd += name;
	objExistsCmd += "\"";

	int exists = 0;
	if (MGlobal::executeCommand(objExistsCmd, exists) != MStatus::kSuccess) return false;

	// annoying warning if just casting
	return (exists != 0);
}
