#include "callbackHandler.h"

#include <maya/MMessage.h>
#include <maya/MUuid.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MTimerMessage.h>
#include "hackprint.h"

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
		MFnDependencyNode node(plug.node());
		std::string uuid = node.uuid().asString().asChar();
		CallbackHandler::getInstance().addNodeToSendList(uuid, std::time(nullptr));
	}
}

void preRemoveCallback(MObject& node, void*)
{
	MFnDependencyNode depNode(node);
	std::string uuid = depNode.uuid().asString().asChar();
	CallbackHandler::getInstance().addNodeToSendList(uuid, std::time(nullptr));
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

std::unordered_map<std::string, std::time_t> CallbackHandler::getSendList()
{
	return sendList;
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

	// TODO couple more things

	return status;
}

MStatus CallbackHandler::registerCallbacksToDetectNewNodes()
{
	return MStatus::kFailure;
}

void CallbackHandler::resetSendList()
{
	sendList.clear();
}

void CallbackHandler::addNodeToSendList(std::string uuid, time_t time)
{
	sendList[uuid] = time;
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



