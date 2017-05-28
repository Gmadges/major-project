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
#include "mayaUtils.h"
#include "dataStore.h"

SendRegister::SendRegister()
	:
	SendAbstract(),
	bTimerOn(false)
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
	if (!DataStore::getInstance().isServerSet())
	{
		HackPrint::print("Set Server using \"SetServer\" command");
		return status;
	}

	pMessaging->resetSocket(DataStore::getInstance().getAddress(), DataStore::getInstance().getPort());

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

		// turn tweaks into a node before sending
		if (registerAndSendMesh(dagPath) != MStatus::kSuccess) return MStatus::kFailure;

		// only gonna handle one mesh for now
		break;
	}

	if (bTimerOn) return MS::kSuccess;

	status = CallbackHandler::getInstance().startTimerCallback();

	if (status == MStatus::kSuccess)
	{
		bTimerOn = true;
	}

	return status;
}

MStatus SendRegister::registerAndSendMesh(MDagPath & meshDAGPath)
{
	MStatus status;

	// grab all nodes and set callbacks

	std::vector<json> nodeList;

	std::function<bool(MFnDependencyNode&)> getNodeAddCallbackFunc = [this, &nodeList](MFnDependencyNode& node) {

		json genNode;
		if (getGenericNode(node, genNode) == MStatus::kSuccess)
		{
			genNode["edit"] = EditType::ADD;

			nodeList.push_back(genNode);
			CallbackHandler::getInstance().registerCallbacksToNode(node.object());
		}
		
		return false;
	};

	traverseAllValidNodesForMesh(meshDAGPath, getNodeAddCallbackFunc);

	CallbackHandler::getInstance().registerCallbacksToDetectNewNodes();

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

	DataStore::getInstance().setCurrentRegisteredMesh(meshData["id"]);

	// transforms name, because i dunno
	meshData["name"] = std::string(transformNode.name().asChar());

	// add user id to the mesh too
	meshData["uid"] = DataStore::getInstance().getUserID();

	// we pass the last node. which should be the first geo node.
	meshData["type"] = MayaUtils::getPolyType(nodeList.back(), status);
	if (status == MStatus::kFailure) return status;

	// add all its nodes
	// minor hack
	// the order of the nodes is going thw wrong way from most recent to older
	std::reverse(nodeList.begin(), nodeList.end());
	meshData["nodes"] = nodeList;

	json message;
	message["uid"] = DataStore::getInstance().getUserID();
	message["requestType"] = ReqType::REGISTER_MESH;
	message["mesh"] = meshData;

	HackPrint::print(MString("sending ") + transformNode.name());
	if (pMessaging->sendUpdate(message))
	{
		HackPrint::print("mesh sent succesfully");
		return MStatus::kSuccess;
	}

	HackPrint::print("unable to send");
	return MStatus::kFailure;
}