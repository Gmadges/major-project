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
		HackPrint::print(plug.info());
	}
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


