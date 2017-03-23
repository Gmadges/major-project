#include "sendUpdate.h"

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnAttribute.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MUuid.h>
#include <maya/MItSelectionList.h>

#include <string>

#include "messaging.h"
#include "tweakHandler.h"
#include "hackPrint.h"
#include "testTypes.h"

#include "callbackHandler.h"
#include "serverAddress.h"

SendUpdate::SendUpdate()
	:
	SendAbstract()
{
}

SendUpdate::~SendUpdate()
{
}

void* SendUpdate::creator()
{
	return new SendUpdate;
}

MStatus	SendUpdate::doIt(const MArgList& args)
{
	// reset socket
	if (!ServerAddress::getInstance().isServerSet())
	{
		HackPrint::print("Set Server using \"SetServer\" command");
		return MStatus::kFailure;
	}

	pMessaging->resetSocket(ServerAddress::getInstance().getAddress(), ServerAddress::getInstance().getPort());


	// get list of things we need to send
	auto list = CallbackHandler::getInstance().getSendList();

	if (list.empty()) return MStatus::kSuccess;
	
	// create node list
	std::vector<json> nodeList;
	
	// need to use a node to find the mesh later
	MObject validNode;

	for (auto& itr : list)
	{
		// get node
		MSelectionList sList;
		MUuid ID(itr.first.c_str());
		sList.add(ID);
		MObject node;
		if (sList.getDependNode(0, node) != MStatus::kSuccess)
		{
			HackPrint::print("nodes been deleted");
			json delNode;
			delNode["id"] = itr.first;
			delNode["time"] = itr.second;
			delNode["edit"] = EditType::DEL;
			nodeList.push_back(delNode);
			continue;
		}

		MFnDependencyNode depNode(node);

		json genNode;
		if (getGenericNode(depNode, genNode) == MStatus::kSuccess)
		{
			genNode["time"] = itr.second;
			genNode["edit"] = EditType::EDIT;
			nodeList.push_back(genNode);
		}

		validNode = node;
	}

	// clear the list
	CallbackHandler::getInstance().resetSendList();

	// if our valid node is still null then we have tried to delete the whole mesh.
	if (validNode.isNull()) return MStatus::kFailure;

	MStatus status;

	// send mesh with node list
	// atm only handling one mesh
	// re-use the last node we dealt with
	MDagPathArray dagArray;
	status = MDagPath::getAllPathsTo(validNode, dagArray);

	if (status != MStatus::kSuccess) return status;

	dagArray[0].extendToShape();

	MFnDependencyNode shapeNode(dagArray[0].node());

	json meshData;
	// use shape nodes id.
	meshData["id"] = std::string(shapeNode.uuid().asString().asChar());

	// add all its nodes
	meshData["nodes"] = nodeList;

	// create message and attach
	json message;
	message["requestType"] = ReqType::NODE_UPDATE;
	message["mesh"] = meshData;

	if (pMessaging->sendUpdate(message))
	{
		HackPrint::print("updates sent");
		return MStatus::kSuccess;
	}

	HackPrint::print("unable to send");
	return MStatus::kFailure;
}

