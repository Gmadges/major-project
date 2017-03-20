#include "callbackCreator.h"

#include <maya/MMessage.h>

#include "hackprint.h"

void nodeChangeCallback(MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void*)
{
	if (msg & MNodeMessage::kConnectionMade)
	{
		HackPrint::print("Connection made ");
	}
	else if (msg & MNodeMessage::kConnectionBroken)
	{
		HackPrint::print("Connection broken ");
	}

	HackPrint::print(plug.info());

	if (msg & MNodeMessage::kOtherPlugSet) {
		HackPrint::print("OtherPlug:" + otherPlug.info());
	}
}


CallbackCreator::CallbackCreator()
{
}


CallbackCreator::~CallbackCreator()
{
	MMessage::removeCallbacks(callbackIds);
}

MStatus CallbackCreator::registerCallbacksToNode(MObject& _node)
{
	MStatus status;

	MCallbackId id = MNodeMessage::addAttributeChangedCallback(_node,
																nodeChangeCallback,
																NULL,
																&status);

	return status;
}

MStatus CallbackCreator::registerCallbacksToDetectNewNodes()
{
	return MStatus::kFailure;
}


