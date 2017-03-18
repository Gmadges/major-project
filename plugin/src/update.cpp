#include "update.h"
#include "hackPrint.h"
#include "messaging.h"

#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MPlug.h"
#include "maya/MArgDatabase.h"
#include "maya/MPlugArray.h"

#include "testTypes.h"
#include "tweakHandler.h"

Update::Update()
	:
	pMessenger(new Messaging("localhost",8080)),
	pTweakHandler(new TweakHandler())
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
	json data;
	
	// if false then we couldnt connect to server
	if (!pMessenger->requestData(data, ReqType::MESH_REQUEST)) return MStatus::kFailure;
	
	// is there actually anything?
	if (data.empty())
	{
		HackPrint::print("Nothing to update");
		return status;
	}

	//test
	HackPrint::print(data.dump(4));

	// check if mesh exists
	std::string meshName = data["name"];
	bool meshexists = doesItExist(MString(meshName.c_str()));

	if (!meshexists)
	{
		HackPrint::print("Mesh doesnt exist");

		// create mesh
		// does nothing atm
		createMesh(data);
	}

	// get nodes
	auto nodeList = data["nodes"];

	for (auto itr : nodeList)
	{
		// check if node exists
		std::string stringName = itr["name"];
		MString nodeName = stringName.c_str();
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

MStatus Update::setNodeValues(json & data)
{
	MSelectionList sList;
	std::string nodeName = data["name"];
	sList.add(MString(nodeName.c_str()));
	MObject node;
	if (sList.getDependNode(0, node) != MStatus::kSuccess) return MStatus::kFailure;
	// rename and set correct details
	MFnDependencyNode depNode(node);

	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	auto dataAttribs = data["attribs"];
	
	return setAttribs(depNode, dataAttribs);
}

MStatus Update::setAttribs(MFnDependencyNode& node, json& attribs)
{
	MStatus status;

	for (json::iterator it = attribs.begin(); it != attribs.end(); ++it)
	{
		MPlug plug = node.findPlug(MString(it.key().c_str()), status);

		if (status == MStatus::kSuccess)
		{
			//TODO object
			//TODO null

			if (it.value().is_boolean())
			{
				plug.setBool(it.value());
				continue;
			}

			if (it.value().is_number_float())
			{
				plug.setFloat(it.value());
				continue;
			}

			if (it.value().is_string())
			{
				std::string val = it.value();
				plug.setString(MString(val.c_str()));
				continue;
			}

			if (it.value().is_number_integer())
			{
				plug.setInt(it.value());
				continue;
			}

			if (it.value().is_number_unsigned())
			{
				plug.setInt64(it.value());
				continue;
			}

			if (it.value().is_array())
			{
				if (it.key().compare("tk") == 0)
				{
					HackPrint::print("got ourselves a tweak");
					std::vector<json> tweakVals = it.value();
					pTweakHandler->setTweakPlugFromArray(plug, tweakVals);
				}

				//TODO all other cases
				continue;
			}

			if (it.value().is_object())
			{
				// TODO
				continue;
			}

			if (it.value().is_null())
			{
				// TODO
				continue;
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

MStatus Update::createMesh(json& _mesh)
{
	MStatus status;

	// create a mesh
	PolyType type = _mesh["type"];

	switch (type)
	{
		case PolyType::CUBE:
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

void Update::renameNodes(MFnDependencyNode & node, json& mesh)
{
	// rename
	// This wont work for multiple 
	for (auto it : mesh["nodes"])
	{
		std::string nodeType = it["type"];
		MString type(nodeType.c_str());
		if (node.typeName() == type)
		{
			std::string nodeName = it["name"];
			node.setName(MString(nodeName.c_str()));
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

MStatus Update::createNode(json& _node)
{
	MStatus status;

	// create a node of same type?
	MString cmd;
	cmd += "createNode \"";
	std::string nodetype = _node["type"];
	cmd += nodetype.c_str();
	cmd += "\"";

	// node name
	cmd += " -n \"";
	std::string nodeName = _node["name"];
	cmd += nodeName.c_str();
	cmd += "\"";

	status = MGlobal::executeCommand(cmd);
	return status;
}

MStatus Update::setConnections(json& _mesh, json& _node)
{
	// if its not a mesh we'll have to wire it in
	std::string type = _node["type"];
	
	if (type.compare("polySplitRing") == 0 ||
		type.compare("polyTweak") == 0 )
	{
		// get mesh and set it to be the one we're effecting
		MSelectionList selList;
		std::string meshName = _mesh["name"];
		selList.add(MString(meshName.c_str()));
		MDagPath dagpath;
		selList.getDagPath(0, dagpath);
		dagpath.extendToShape();
		setMeshNode(dagpath);

		// get node and do the connections
		MSelectionList sList;
		std::string nodeName = _node["name"];
		sList.add(MString(nodeName.c_str()));
		MObject node;
		if (sList.getDependNode(0, node) != MStatus::kSuccess) return MStatus::kFailure;

		//// and add it to the DAG
		doModifyPoly(node);

		// this is if we require extra connections
		if (type.compare("polySplitRing") == 0)
		{
			MString connectCmd;
			connectCmd += "connectAttr ";
			std::string meshName = _mesh["name"];
			connectCmd += meshName.c_str();
			connectCmd += ".worldMatrix[0] ";
			std::string nodeName = _node["name"];
			connectCmd += nodeName.c_str();
			connectCmd += ".manipMatrix;";
			MGlobal::executeCommand(connectCmd);
		}
	}

	return MStatus::kSuccess;
}

bool Update::doesItExist(MString& name)
{
	// TODO needs rewrite to work with UUIDS

	MString objExistsCmd;
	objExistsCmd += "objExists \"";
	objExistsCmd += name;
	objExistsCmd += "\"";

	int exists = 0;
	if (MGlobal::executeCommand(objExistsCmd, exists) != MStatus::kSuccess) return false;

	// annoying warning if just casting
	return (exists != 0);
}
