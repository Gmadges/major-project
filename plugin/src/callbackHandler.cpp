#include "callbackHandler.h"

#include <maya/MMessage.h>
#include <maya/MUuid.h>
#include <maya/MFnDependencyNode.h>

#include "hackprint.h"

#include <ctime>
#include <string>

void nodeChangeCallback(MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void*)
{
	if (msg & MNodeMessage::kAttributeSet ||
		msg & MNodeMessage::kAttributeRemoved ||
		msg & MNodeMessage::kAttributeRenamed ||
		msg & MNodeMessage::kAttributeAdded ||
		msg & MNodeMessage::kAttributeArrayAdded ||
		msg & MNodeMessage::kAttributeArrayRemoved)
	{
		HackPrint::print("change");
		HackPrint::print(plug.info());

		MFnDependencyNode node(plug.node());
		std::string uuid = node.uuid().asString().asChar();
		CallbackHandler::getInstance().addNodeToSendList(uuid, std::time(nullptr));
	}
}

void preRemoveCallback(MObject& node, void*)
{
	HackPrint::print("pre-remove");
	HackPrint::print(node.apiTypeStr());
	MFnDependencyNode depNode(node);
	std::string uuid = depNode.uuid().asString().asChar();
	CallbackHandler::getInstance().addNodeToSendList(uuid, std::time(nullptr));
}

CallbackHandler::~CallbackHandler()
{
	MMessage::removeCallbacks(callbackIds);
}

MStatus CallbackHandler::registerCallbacksToNode(MObject& _node)
{
	//TESTING
	for (auto& it : sendList)
	{
		std::string val;
		val += it.first;
		val += " : "; 
		val += std::to_string(it.second);
		HackPrint::print(val);
	}


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

	// TODO added stuff

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



