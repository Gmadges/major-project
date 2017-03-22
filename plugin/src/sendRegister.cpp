#include "sendRegister.h"

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MString.h>
#include <maya/MFnMesh.h>
#include <maya/MArgList.h>
#include <maya/MPoint.h>
#include <maya/MIOStream.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnAttribute.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MUuid.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>

#include "messaging.h"
#include "tweakHandler.h"
#include "hackPrint.h"
#include "testTypes.h"

#include "callbackHandler.h"

SendRegister::SendRegister()
	:
	SendAbstract()
{
}

SendRegister::~SendRegister()
{
}

void* SendRegister::creator()
{
	return new SendRegister;
}

MStatus	SendRegister::doIt(const MArgList& args)
{
	MStatus status;

	// reset socket
	MString addr;
	int port;
	status = getArgs(args, addr, port);
	if (status != MStatus::kSuccess)
	{
		HackPrint::print("no input values specified");
		return status;
	}

	pMessaging->resetSocket(std::string(addr.asChar()), port);

	MSelectionList selList;
	MGlobal::getActiveSelectionList(selList);
	MItSelectionList selListIter(selList);
	selListIter.setFilter(MFn::kMesh);

	// check any meshes are actually selected
	if (selListIter.isDone())
	{
		HackPrint::print("no mesh selected!");
		return MStatus::kFailure;
	}

	for (; !selListIter.isDone(); selListIter.next())
	{

		MDagPath dagPath;

		status = selListIter.getDagPath(dagPath);

		if (!status) {
			status.perror("MItDag::getPath");
			continue;
		}
		MFnDagNode dagNode(dagPath, &status);
		if (!status) {
			status.perror("MFnDagNode constructor");
			continue;
		}

		// check for tweaks
		if (pTweaksHandler->hasTweaks(dagPath))
		{
			MObject tweakNode;
			if (pTweaksHandler->createPolyTweakNode(dagPath, tweakNode) == MStatus::kSuccess)
			{
				dagPath.extendToShape();
				pTweaksHandler->connectTweakNodes(tweakNode, dagPath.node());
			}
		}

		// turn tweaks into a node before sending
		if (registerAndSendMesh(dagPath) != MStatus::kSuccess) return MStatus::kFailure;

		// only gonna handle one mesh for now
		break;
	}
	return MS::kSuccess;
}

MStatus SendRegister::registerAndSendMesh(MDagPath & meshDAGPath)
{
	MStatus status;

	json message;
	message["requestType"] = ReqType::MESH_UPDATE;

	// grab all nodes and set callbacks

	std::vector<json> nodeList;

	std::function<void(MFnDependencyNode&)> getNodeAddCallbackFunc = [this, &nodeList](MFnDependencyNode& node) {

		json genNode;
		if (getGenericNode(node, genNode) == MStatus::kSuccess)
		{
			nodeList.push_back(genNode);
			//test
			CallbackHandler::getInstance().registerCallbacksToNode(node.object());
		}
	};

	traverseAllValidNodesForMesh(meshDAGPath, getNodeAddCallbackFunc);

	// should have atleast 3 nodes for a mesh
	// transform, shape and mesh
	if (nodeList.size() < 3) return MStatus::kFailure;

	json meshData;

	// names are slightly confusing
	// we use the shape nodes ID
	// but its knows by the transforms name
	// first do the transforms node
	MFnDependencyNode transformNode(meshDAGPath.transform());
	meshDAGPath.extendToShape();
	MFnDependencyNode meshShapeNode(meshDAGPath.node());

	// use shape nodes id.
	meshData["id"] = std::string(meshShapeNode.uuid().asString().asChar());

	// transforms name, because i dunno
	meshData["name"] = std::string(transformNode.name().asChar());

	// hardcode cube for now
	meshData["type"] = PolyType::CUBE;

	// add all its nodes
	// minor hack
	// the order of the nodes is going thw wrong way from most recent to older
	std::reverse(nodeList.begin(), nodeList.end());
	meshData["nodes"] = nodeList;

	// attach the mesh to the message
	message["mesh"] = meshData;

	HackPrint::print("sending " + transformNode.name());
	if (pMessaging->sendUpdate(message))
	{
		HackPrint::print("mesh sent succesfully");
		return MStatus::kSuccess;
	}

	HackPrint::print("unable to send");
	return MStatus::kFailure;
}