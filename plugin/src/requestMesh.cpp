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
#include "dataStore.h"
#include "callbackHandler.h"
#include "mayaUtils.h"

RequestMesh::RequestMesh()
	:
	pMessenger(new Messaging("localhost",8080))
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

MStatus	RequestMesh::doIt(const MArgList& args)
{
	MStatus status = MStatus::kSuccess;

	// reset socket
	if (!DataStore::getInstance().isServerSet())
	{
		HackPrint::print("Set Server using \"SetServer\" command");
		return status;
	}

	pMessenger->resetSocket(DataStore::getInstance().getAddress(), DataStore::getInstance().getPort());

	// ask the server for any update
	json data;
	MString id;
	if (getArgs(args, id) != MStatus::kSuccess)
	{
		HackPrint::print("no id specified!");
		return MStatus::kFailure;
	}

	// if false then we couldnt connect to server
	if (!pMessenger->requestMesh(data, ReqType::REQUEST_MESH, std::string(id.asChar()), DataStore::getInstance().getUserID())) return MStatus::kFailure;
	
	// is there actually anything?
	if (data.empty())
	{
		HackPrint::print("Nothing to update");
		return status;
	}

	// check if mesh exists
	std::string meshID = data["id"];
	bool meshexists = MayaUtils::doesItExist(meshID);

	if (!meshexists)
	{
		HackPrint::print("creating mesh");
		// create mesh
		// does nothing atm
		createMesh(data);
	}

	// set this as our current mesh now
	DataStore::getInstance().setCurrentRegisteredMesh(meshID);

	// get nodes
	auto nodeList = data["nodes"];

	// new work flow

	// create nodes
	for (auto& itr : nodeList)
	{
		std::string stringID = itr["id"];
		bool nodeExists = MayaUtils::doesItExist(stringID);
		
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
		status = setConnections(itr);
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

MStatus RequestMesh::createMesh(json& _mesh)
{
	MStatus status;

	// create a mesh
	PolyType type = _mesh["type"];
	MString cmd;

	switch (type)
	{
		case PolyType::CUBE:
		{
			cmd = "polyCube";
			break;
		}
		case PolyType::CONE:
		{
			cmd = "polyCone";
			break;
		}
		case PolyType::CYLINDER:
		{
			cmd = "polyCylinder";
			break;
		}
		case PolyType::PIPE:
		{
			cmd = "polyPipe";
			break;
		}
		case PolyType::SPHERE:
		{
			cmd = "polySphere";
			break;
		}
		case PolyType::PLANE:
		{
			cmd = "polyPlane";
			break;
		}
		case PolyType::PYRAMID:
		{
			cmd = "polyPyramid";
			break;
		}
		case PolyType::TORUS:
		{
			cmd = "polyTorus";
			break;
		}
		default : 
		{
			return MStatus::kFailure;
		}
	}

	MStringArray result;
	HackPrint::print(cmd);
	status = MGlobal::executeCommand(cmd, result);
	if (status != MStatus::kSuccess) return status;
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
	inMeshPlug = MayaUtils::getInPlug(node, status);

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
