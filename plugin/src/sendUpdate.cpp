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
#include "hackPrint.h"
#include "testTypes.h"
#include "mayaUtils.h"

#include "callbackHandler.h"
#include "dataStore.h"

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
	if (!DataStore::getInstance().isServerSet())
	{
		HackPrint::print("Set Server using \"SetServer\" command");
		return MStatus::kFailure;
	}

	pMessaging->resetSocket(DataStore::getInstance().getAddress(), DataStore::getInstance().getPort());

	// we do this here because it might get reset in the delete node method.
	std::string currentID = DataStore::getInstance().getCurrentRegisteredMesh();

	std::vector<json> nodeList;

	// this order is vaguely important we wanna add then update then del in databse i think
	processNewNodes(nodeList);

	processEditedNodes(nodeList);

	processDeletedNodes(nodeList);

	if (nodeList.empty()) return MStatus::kSuccess;

	// if our valid node is still null then we have tried to delete the whole mesh.
	if (DataStore::getInstance().getCurrentRegisteredMesh().empty()) return MStatus::kFailure;

	json meshData;
	// use shape nodes id.
	meshData["id"] = currentID;

	// add all its nodes
	meshData["nodes"] = nodeList;

	// create message and attach
	json message;
	message["uid"] = DataStore::getInstance().getUserID();
	message["requestType"] = ReqType::MESH_UPDATE;
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
	auto list = DataStore::getInstance().getAddedList();

	if (list.empty()) return;

	for (auto& itr : list)
	{
		MObject node;
		MString id = itr.first.c_str();
		if (MayaUtils::getNodeObjectFromUUID(id, node) != MStatus::kSuccess) continue;

		// check its a node for the correct mesh
		if(!isNodeFromRegisteredMesh(node)) continue;

		// register this node with callbacks
		CallbackHandler::getInstance().registerCallbacksToNode(node);

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
	DataStore::getInstance().resetAddedList();
}

void SendUpdate::processEditedNodes(std::vector<json>& nodeList)
{
	// get list of things we need to send
	auto list = DataStore::getInstance().getEditsList();

	if (list.empty()) return;

	for (auto& itr : list)
	{
		MObject node;
		MString id = itr.first.c_str();
		if (MayaUtils::getNodeObjectFromUUID(id, node) != MStatus::kSuccess) continue;

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
	DataStore::getInstance().resetEditList();
}

void SendUpdate::processDeletedNodes(std::vector<json>& nodeList)
{
	// get list of things we need to send
	auto list = DataStore::getInstance().getDeletedList();

	if (list.empty()) return;

	std::vector<json> deleteList;

	std::string regID = DataStore::getInstance().getCurrentRegisteredMesh();

	for (auto& itr : list)
	{
		//if the ide is the same as our registered id then thats bad
		if (regID.compare(itr.first) == 0)
		{
			// very important
			// we're deleting our main mesh thing but still wanna let it send edits and stuff

			// we reset the list
			DataStore::getInstance().resetDeleteList();
			//clear callbacks
			CallbackHandler::getInstance().clearCallbacks();
			// clear current registered mesh
			DataStore::getInstance().setCurrentRegisteredMesh("");
			// stop here we're done, not sending anything to be deleted.
			return;

			// TODO find a way of telling gui whats happened
		}

		json delNode;
		delNode["id"] = itr.first;
		delNode["time"] = itr.second;
		delNode["edit"] = EditType::DEL;
		deleteList.push_back(delNode);
	}

	nodeList.insert(nodeList.end(), deleteList.begin(), deleteList.end());

	// clear the list
	DataStore::getInstance().resetDeleteList();
}

bool SendUpdate::isNodeFromRegisteredMesh(MObject& _node)
{
	MStatus status;

	MFnDependencyNode depNode(_node);
	MString currentMeshID = DataStore::getInstance().getCurrentRegisteredMesh().c_str();
	MUuid id = depNode.uuid();

	if (id.asString() != currentMeshID)
	{
		// TODO
		// a more efficient method of doing this.

		bool isConnected = false;

		std::function<bool(MFnDependencyNode&)> checkOurIDFunc = [this, &isConnected, &id](MFnDependencyNode& node) {

			if (id == node.uuid())
			{
				isConnected = true;
			}

			return isConnected;
		};

		MDagPath dagPath;
		MObject tmp;
		status = MayaUtils::getNodeObjectFromUUID(currentMeshID, tmp);
		if (status != MStatus::kSuccess) return status;
		status = MDagPath::getAPathTo(tmp, dagPath);
		status = dagPath.extendToShape();

		traverseAllValidNodesForMesh(dagPath, checkOurIDFunc);

		return isConnected;
	}

	return true;
}

