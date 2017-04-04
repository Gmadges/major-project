#include "requestMesh.h"
#include "hackPrint.h"
#include "messaging.h"

#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"
#include "maya/MDagPathArray.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MPlug.h"
#include "maya/MArgDatabase.h"
#include "maya/MPlugArray.h"
#include "maya/MUuid.h"

#include "testTypes.h"
#include "tweakHandler.h"
#include "serverAddress.h"
#include "callbackHandler.h"
#include "mayaUtils.h"

RequestMesh::RequestMesh()
	:
	pMessenger(new Messaging("localhost",8080)),
	pTweakHandler(new TweakHandler())
{
}

RequestMesh::~RequestMesh()
{
}

void* RequestMesh::creator()
{
	return new RequestMesh;
}

MSyntax RequestMesh::newSyntax()
{
	MSyntax syn;

	syn.addFlag("-id", "-uuid", MSyntax::kString);

	return syn;
}

MStatus	RequestMesh::doIt(const MArgList& args)
{
	MStatus status = MStatus::kSuccess;

	// reset socket
	if (!ServerAddress::getInstance().isServerSet())
	{
		HackPrint::print("Set Server using \"SetServer\" command");
		return status;
	}

	pMessenger->resetSocket(ServerAddress::getInstance().getAddress(), ServerAddress::getInstance().getPort());

	// ask the server for any update
	json data;
	MString id;
	if (getArgs(args, id) != MStatus::kSuccess)
	{
		HackPrint::print("no id specified!");
		return MStatus::kFailure;
	}

	// if false then we couldnt connect to server
	if (!pMessenger->requestMesh(data, ReqType::MESH_REQUEST, std::string(id.asChar()))) return MStatus::kFailure;
	
	// is there actually anything?
	if (data.empty())
	{
		HackPrint::print("Nothing to update");
		return status;
	}

	// check if mesh exists
	std::string meshID = data["id"];
	bool meshexists = doesItExist(meshID);

	if (!meshexists)
	{
		HackPrint::print("creating mesh");
		// create mesh
		// does nothing atm
		createMesh(data);
	}

	// set this as our current mesh now
	CallbackHandler::getInstance().setCurrentRegisteredMesh(meshID);

	// get nodes
	auto nodeList = data["nodes"];

	// new work flow

	// create nodes
	for (auto& itr : nodeList)
	{
		std::string stringID = itr["id"];
		bool nodeExists = doesItExist(stringID);
		
		if (!nodeExists)
		{
			HackPrint::print("Creating Node: " + itr["name"].get<std::string>());
			status = createNode(itr);
		}
	}

	// set values
	for (auto& itr : nodeList)
	{
		status = setNodeValues(itr);
	}

	// set and check connections
	for (auto& itr : nodeList)
	{
		status = setConnections(data, itr);
	}

	// set callbacks
	for (auto& itr : nodeList)
	{
		MObject node;
		MString id = itr["id"].get<std::string>().c_str();
		status = MayaUtils::getNodeObjectFromUUID(id, node);
		CallbackHandler::getInstance().registerCallbacksToNode(node);
	}

	// register other callbacks
	CallbackHandler::getInstance().registerCallbacksToDetectNewNodes();
	CallbackHandler::getInstance().startTimerCallback();

	return status;
}

MStatus RequestMesh::setNodeValues(json & _node)
{
	MObject node;
	MString id = _node["id"].get<std::string>().c_str();
	if (MayaUtils::getNodeObjectFromUUID(id, node) != MStatus::kSuccess) return MStatus::kFailure;
	// rename and set correct details
	MFnDependencyNode depNode(node);

	depNode.setName(_node["name"].get<std::string>().c_str());

	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	auto dataAttribs = _node["attribs"];
	
	return setAttribs(depNode, dataAttribs);
}

MStatus RequestMesh::setAttribs(MFnDependencyNode& node, json& attribs)
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

MStatus RequestMesh::getArgs(const MArgList& args, MString& id)
{
	MStatus status = MStatus::kSuccess;
	MArgDatabase parser(syntax(), args, &status);

	if (status != MS::kSuccess) return status;

	if (parser.isFlagSet("-id"))
	{
		parser.getFlagArgument("-id", 0, id);
	}
	else
	{
		status = MStatus::kFailure;
	}

	return status;
}

MStatus RequestMesh::createMesh(json& _mesh)
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
			MDagPath dagPath;
			MDagPath::getAPathTo(node, dagPath);
			dagPath.extendToShape();

			MFnDependencyNode shapeNode(dagPath.node());
			matchIDs(shapeNode, _mesh);

			//transform node
			MFnDependencyNode tranformNode(node);
			matchIDs(tranformNode, _mesh);
			
			return status;
		}
	}

	return MStatus::kFailure;
}

void RequestMesh::matchIDs(MFnDependencyNode & node, json& mesh)
{
	// we should really only have 3 nodes because we just made a fresh one
	// so we're only reset the ID's for the shape, transform and polymesh
	for (auto it : mesh["nodes"])
	{
		std::string nodeType = it["type"];
		MString type(nodeType.c_str());
		MString tmp = node.typeName();

		if (node.typeName() == type)
		{
			MUuid tmpID(it["id"].get<std::string>().c_str());	
			node.setUuid(tmpID);

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

		matchIDs(upstreamNode, mesh);
	}
}

MStatus RequestMesh::createNode(json& _node)
{
	MStatus status;

	std::string nodetype = _node["type"];
	MObject obj = fDGModifier.createNode(MString(nodetype.c_str()), &status);

	if (status != MStatus::kSuccess) return status;

	MFnDependencyNode node;
	node.setObject(obj);
	node.setName(_node["name"].get<std::string>().c_str());
	MUuid id(_node["id"].get<std::string>().c_str());
	node.setUuid(id);

	return fDGModifier.doIt();
}

MStatus RequestMesh::setConnections(json& _mesh, json& _node)
{
	MStatus status;

	// TODO check that the nodes in and out match correctly

	// if its not a mesh we'll have to wire it in
	std::string type = _node["type"];

	if (type.compare("polySplitRing") == 0 ||
		type.compare("polyTweak") == 0 )
	{
		// get mesh and set it to be the one we're effecting
		MString meshID(_mesh["id"].get<std::string>().c_str());
		MObject meshNode;
		status = MayaUtils::getNodeObjectFromUUID(meshID, meshNode);
		if(status != MStatus::kSuccess) return status;

		MDagPath dagPath;
		MDagPath::getAPathTo(meshNode, dagPath);
		dagPath.extendToShape();
		setMeshNode(dagPath);

		// get node and do the connections
		MString nodeID(_node["id"].get<std::string>().c_str());
		MObject node;
		status = MayaUtils::getNodeObjectFromUUID(nodeID, node);
		if (status != MStatus::kSuccess) return status;

		//// and add it to the DAG
		status = doModifyPoly(node);
		if (status != MStatus::kSuccess) return status;

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

bool RequestMesh::doesItExist(std::string& _id)
{
	MStatus status;
	MSelectionList selList;
	MUuid id(_id.c_str());
	status = selList.add(id);

	return (status == MStatus::kSuccess);
}