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
	MGlobal::executeCommandOnIdle("SendUpdates");
}


//////////////////////////class methods

CallbackHandler::~CallbackHandler()
{
	MMessage::removeCallbacks(callbackIds);
}

MStatus CallbackHandler::registerCallbacksToNode(MObject& _node)
{
	MStatus status;

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

	// TODO Name Change

	// TODO added nodes

	// TODO attribute added or removed 

	return status;
}

MStatus CallbackHandler::registerCallbacksToDetectNewNodes()
{
	MStatus status;

	MCallbackId id = MDGMessage::addNodeAddedCallback(newNodeCallback,
															"dependNode",
															NULL,
															&status);
	if (status == MStatus::kSuccess)
	{
		callbackIds.append(id);
	}

	return MStatus::kFailure;
}

MStatus CallbackHandler::startTimerCallback()
{
	MStatus status;

	MCallbackId id = MTimerMessage::addTimerCallback(2.0f,
													timerCallback,
													NULL,
													&status);

	if (status == MStatus::kSuccess)
	{
		callbackIds.append(id);
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
	delList[uuid] = time;
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
	addList[uuid] = time;
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
	editList[uuid] = time;
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