#include "callbackHandler.h"

#include <maya/MMessage.h>
#include <maya/MUuid.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MTimerMessage.h>
#include <maya/MDagMessage.h>
#include <maya/MDGMessage.h>
#include "hackprint.h"
#include "mayaUtils.h"

#include <ctime>
#include <string>

///////////////////////////// callbacks

void nodeChangeCallback(MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void*)
{
	if (msg & MNodeMessage::kAttributeSet ||
		msg & MNodeMessage::kAttributeRemoved ||
		msg & MNodeMessage::kAttributeRenamed ||
		msg & MNodeMessage::kAttributeAdded ||
		msg & MNodeMessage::kAttributeArrayAdded ||
		msg & MNodeMessage::kAttributeArrayRemoved)
	{
		// not bothered about cache inputs
		if (plug.partialName() == "cin") return;

		std::string test = plug.info().asChar();

		MFnDependencyNode node(plug.node());
		std::string uuid = node.uuid().asString().asChar();
		CallbackHandler::getInstance().addNodeToEditList(uuid, std::time(nullptr));
	}
}

void preRemoveCallback(MObject& node, void*)
{
	MFnDependencyNode depNode(node);
	std::string uuid = depNode.uuid().asString().asChar();
	CallbackHandler::getInstance().addNodeToDeleteList(uuid, std::time(nullptr));
}


void uuidChangeCallback(MObject &node, const MUuid &uuid, void *clientData)
{
	// TODO
	// find a nice way of handling uuid changes
	//MFnDependencyNode depNode(node);
	//std::string uuid = depNode.uuid().asString().asChar();
	//CallbackHandler::getInstance().addNodeToDeleteList(uuid, std::time(nullptr));
}

void nodeNameChangeCallback(MObject &node, const MString &str, void *clientData)
{
	MFnDependencyNode depNode(node);
	std::string uuid = depNode.uuid().asString().asChar();
	CallbackHandler::getInstance().addNodeToDeleteList(uuid, std::time(nullptr));
}

void nodeAttribAddCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, void *clientData)
{
	if (msg & MNodeMessage::kAttributeSet ||
		msg & MNodeMessage::kAttributeRemoved ||
		msg & MNodeMessage::kAttributeRenamed ||
		msg & MNodeMessage::kAttributeAdded ||
		msg & MNodeMessage::kAttributeArrayAdded ||
		msg & MNodeMessage::kAttributeArrayRemoved)
	{
		// not bothered about cache inputs
		if (plug.partialName() == "cin") return;

		std::string test = plug.info().asChar();

		MFnDependencyNode node(plug.node());
		std::string uuid = node.uuid().asString().asChar();
		CallbackHandler::getInstance().addNodeToEditList(uuid, std::time(nullptr));
	}
}

void newNodeCallback(MObject &node, void *clientData)
{
	MFnDependencyNode depNode(node);
	if (MayaUtils::isValidNodeType(depNode.typeName()))
	{
		std::string uuid = depNode.uuid().asString().asChar();
		CallbackHandler::getInstance().addNodeToAddedList(uuid, std::time(nullptr));
	}
}


void timerCallback(float elapsedTime, float lastTime, void *clientData)
{
	if (CallbackHandler::getInstance().anyChanges())
	{
		MGlobal::executeCommandOnIdle("SendUpdates");
	}
	else
	{
		MGlobal::executeCommandOnIdle("RequestUpdate");
	}
}

//////////////////////////class methods

CallbackHandler::CallbackHandler()
	:
	timerCallbackEnabled(false),
	newNodeCallbackEnabled(false),
	bIgnoreChanges(false)
{
}


CallbackHandler::~CallbackHandler()
{
	MMessage::removeCallbacks(callbackIds);
}

MStatus CallbackHandler::clearCallbacks()
{
	MStatus status = MMessage::removeCallbacks(callbackIds);
	callbackIds.clear();
	timerCallbackEnabled = false;
	newNodeCallbackEnabled = false;
	return status;
}

MStatus CallbackHandler::registerCallbacksToNode(MObject& _node)
{
	MStatus status;

	// wipe the callbacks off a node if its already has them
	status = cleanNodeOfCallbacks(_node);

	MCallbackId id = MNodeMessage::addAttributeChangedCallback(_node,
																nodeChangeCallback,
																NULL,
																&status);

	if (status == MStatus::kSuccess)
	{
		callbackIds.append(id);
	}


	id = MNodeMessage::addNodePreRemovalCallback(_node,
													preRemoveCallback,
													NULL,
													&status);

	if (status == MStatus::kSuccess)
	{
		callbackIds.append(id);
	}

	id = MNodeMessage::addUuidChangedCallback( _node, 
												uuidChangeCallback, 
												NULL, 
												&status);

	if (status == MStatus::kSuccess)
	{
		callbackIds.append(id);
	}

	id = MNodeMessage::addNameChangedCallback( _node,
												nodeNameChangeCallback,
												NULL,
												&status);
	
	if (status == MStatus::kSuccess)
	{
		callbackIds.append(id);
	}

	id = MNodeMessage::addAttributeAddedOrRemovedCallback( _node,
															nodeAttribAddCallback,
															NULL,
															&status);

	if (status == MStatus::kSuccess)
	{
		callbackIds.append(id);
	}

	return status;
}

MStatus CallbackHandler::registerCallbacksToDetectNewNodes()
{
	if (newNodeCallbackEnabled) return MStatus::kSuccess;

	MStatus status;

	MCallbackId id = MDGMessage::addNodeAddedCallback(newNodeCallback,
															"dependNode",
															NULL,
															&status);
	if (status == MStatus::kSuccess)
	{
		callbackIds.append(id);
		newNodeCallbackEnabled = true;
	}

	return status;
}

MStatus CallbackHandler::startTimerCallback()
{
	if (timerCallbackEnabled) return MStatus::kSuccess;
	
	MStatus status;

	MCallbackId id = MTimerMessage::addTimerCallback(2.0f,
													timerCallback,
													NULL,
													&status);

	if (status == MStatus::kSuccess)
	{
		callbackIds.append(id);
		timerCallbackEnabled = true;
	}

	return status;
}

MStatus CallbackHandler::cleanNodeOfCallbacks(MObject& _node)
{
	MCallbackIdArray nodeCallbacks;

	MStatus status = MMessage::nodeCallbacks(_node, nodeCallbacks);

	// remove the callbacks 
	
	for (unsigned int i = 0; i < nodeCallbacks.length(); i++)
	{
		// remove the callback in general
		status = MMessage::removeCallback(nodeCallbacks[i]);

		// remove the id from our list
		for (unsigned int j = 0; i < callbackIds.length(); i++)
		{
			if (nodeCallbacks[i] == callbackIds[i])
			{
				callbackIds.remove(j);
				break;
			}
		}
	}

	return status;
}

// deletes
std::unordered_map<std::string, std::time_t> CallbackHandler::getDeletedList()
{
	return delList;
}

void CallbackHandler::addNodeToDeleteList(std::string uuid, time_t time)
{
	if (!bIgnoreChanges)
	{
		delList[uuid] = time;
	}
}

void CallbackHandler::resetDeleteList()
{
	delList.clear();
}

// adds
std::unordered_map<std::string, std::time_t> CallbackHandler::getAddedList()
{
	return addList;
}

void CallbackHandler::addNodeToAddedList(std::string uuid, time_t time)
{
	if (!bIgnoreChanges)
	{
		addList[uuid] = time;
	}
}

void CallbackHandler::resetAddedList()
{
	addList.clear();
}

// edits
std::unordered_map<std::string, std::time_t> CallbackHandler::getEditsList()
{
	return editList;
}

void CallbackHandler::addNodeToEditList(std::string uuid, time_t time)
{
	if (!bIgnoreChanges)
	{
		editList[uuid] = time;
	}
}

void CallbackHandler::resetEditList()
{
	editList.clear();
}

std::string CallbackHandler::getCurrentRegisteredMesh()
{
	return currentMeshID;
}

void CallbackHandler::setCurrentRegisteredMesh(std::string meshID)
{
	currentMeshID = meshID;
}

bool CallbackHandler::anyChanges()
{
	return (!addList.empty() ||
			!editList.empty() ||
			!delList.empty());
}

void CallbackHandler::ignoreChanges(bool ignore)
{
	bIgnoreChanges = ignore;
}