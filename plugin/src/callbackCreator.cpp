#include "callbackCreator.h"

#include <maya/MMessage.h>
#include "hackprint.h"

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
	}
}

void dirtyNodeCallback(MObject& node, MPlug & plug, void*)
{
	HackPrint::print("dirty");
	HackPrint::print(plug.info());
}

void preRemoveCallback(MObject& node, void*)
{
	HackPrint::print("pre-remove");
	HackPrint::print(node.apiTypeStr());
}

void nodeRemovedCallback(void*)
{
	HackPrint::print("node destroyed");
}

CallbackCreator::~CallbackCreator()
{
	removeCallbacks();
}

MStatus CallbackCreator::registerCallbacksToNode(MObject& _node)
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

	// add a node removal callback

	id = MNodeMessage::addNodeDestroyedCallback(_node,
												nodeRemovedCallback,
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

	id = MNodeMessage::addNodeDirtyPlugCallback(_node,
												dirtyNodeCallback,
												NULL,
												&status);

	if (status == MStatus::kSuccess)
	{
		callbackIds.append(id);
	}

	return status;
}

MStatus CallbackCreator::registerCallbacksToDetectNewNodes()
{
	return MStatus::kFailure;
}

void CallbackCreator::removeCallbacks()
{
	MMessage::removeCallbacks(callbackIds);
}


