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

	std::vector<json> nodeList;

	// this order is vaguely important we wanna add then update then del in databse i think
	processNewNodes(nodeList);

	processEditedNodes(nodeList);

	processDeletedNodes(nodeList);

	// if our valid node is still null then we have tried to delete the whole mesh.
	if (CallbackHandler::getInstance().getCurrentRegisteredMesh().empty()) return MStatus::kFailure;

	json meshData;
	// use shape nodes id.
	meshData["id"] = CallbackHandler::getInstance().getCurrentRegisteredMesh();

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

void SendUpdate::processNewNodes(std::vector<json>& nodeList)
{
	// get list of things we need to send
	auto list = CallbackHandler::getInstance().getAddedList();

	if (list.empty()) return;

	for (auto& itr : list)
	{
		// get node
		MSelectionList sList;
		MUuid ID(itr.first.c_str());
		sList.add(ID);
		MObject node;
		if (sList.getDependNode(0, node) != MStatus::kSuccess) continue;

		MFnDependencyNode depNode(node);

		json genNode;
		if (getGenericNode(depNode, genNode) == MStatus::kSuccess)
		{
			genNode["time"] = itr.second;
			genNode["edit"] = EditType::ADD;
			nodeList.push_back(genNode);
		}
	}

	// clear the list
	CallbackHandler::getInstance().resetAddedList();
}

void SendUpdate::processEditedNodes(std::vector<json>& nodeList)
{
	// get list of things we need to send
	auto list = CallbackHandler::getInstance().getEditsList();

	if (list.empty()) return;

	for (auto& itr : list)
	{
		// get node
		MSelectionList sList;
		MUuid ID(itr.first.c_str());
		sList.add(ID);
		MObject node;
		if (sList.getDependNode(0, node) != MStatus::kSuccess) continue;

		MFnDependencyNode depNode(node);

		json genNode;
		if (getGenericNode(depNode, genNode) == MStatus::kSuccess)
		{
			genNode["time"] = itr.second;
			genNode["edit"] = EditType::EDIT;
			nodeList.push_back(genNode);
		}
	}

	// clear the list
	CallbackHandler::getInstance().resetEditList();
}

void SendUpdate::processDeletedNodes(std::vector<json>& nodeList)
{
	// get list of things we need to send
	auto list = CallbackHandler::getInstance().getDeletedList();

	if (list.empty()) return;

	for (auto& itr : list)
	{
		json delNode;
		delNode["id"] = itr.first;
		delNode["time"] = itr.second;
		delNode["edit"] = EditType::DEL;
		nodeList.push_back(delNode);
	}

	// clear the list
	CallbackHandler::getInstance().resetDeleteList();
}

