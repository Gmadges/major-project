#include "update.h"
#include "hackPrint.h"
#include "messaging.h"

#include "genericMeshMessage.h"

#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MPlug.h"

Update::Update()
	:
	pMessenger(new Messaging("8080"))
{
}

Update::~Update()
{
}

void* Update::creator()
{
	return new Update;
}

MStatus	Update::doIt(const MArgList& args)
{
	MStatus status = MStatus::kSuccess;

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

	for ( auto atr : dataAttribs )
	{
		//if (atr.first.compare("out") == 0 || atr.first.compare("ip") == 0) continue;
		//if (atr.first.compare("axx") != 0 ) continue;

		MPlug plug = depNode.findPlug(atr.first.c_str(), status);

		if (status == MStatus::kSuccess)
		{
			//HackPrint::print(plug.name());

			switch (atr.second.type)
			{
				case msgpack::type::BOOLEAN:
				{
					//HackPrint::print("bool");
					plug.setBool(atr.second.via.boolean);
					break;
				}
				case msgpack::type::FLOAT:
				{
					//HackPrint::print("float");
					//HackPrint::print(std::to_string(atr.second.via.f64));
					plug.setFloat(atr.second.via.f64);
					break;
				}
				case msgpack::type::STR :
				{
					std::string val;
					atr.second.convert(val);
					//HackPrint::print(val);
					plug.setString(MString(val.c_str()));
					break;
				}
				case msgpack::type::NEGATIVE_INTEGER:
				{
					//HackPrint::print("Neg Int");
					int val;
					atr.second.convert(val);
					//HackPrint::print(std::to_string(val));
					plug.setInt(val);
					break;
				}
				case msgpack::type::POSITIVE_INTEGER:
				{
					//HackPrint::print("Pos int");
					//HackPrint::print(std::to_string(atr.second.via.i64));
					plug.setInt64(atr.second.via.i64);
					break;
				}
			}
		}
	}

	return MStatus::kSuccess;
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
			cmd += "polyCube ";

			// mesh name
			cmd += " -n \"";
			cmd += _mesh.getMeshName().c_str();
			cmd += "\"";

			HackPrint::print(cmd);
			status = MGlobal::executeCommand(cmd);
			return status;
		}
	}

	return MStatus::kFailure;
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
	if (_node.getNodeType().compare("polySplitRing") == 0)
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
