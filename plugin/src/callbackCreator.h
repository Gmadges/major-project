#ifndef CALLBACKCREATOR_H
#define CALLBACKCREATOR_H

#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MCallbackIdArray.h>
#include <maya/MNodeMessage.h>
#include <maya/MPlug.h>

class CallbackCreator
{
public:
	CallbackCreator();
	~CallbackCreator();

	// registers a callback to look at attribute changes
	// and another for its deletion
	MStatus registerCallbacksToNode(MObject& _node);
	MStatus registerCallbacksToDetectNewNodes();

private:
	//callback methods
	//void nodeChangeCallback(MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void*);

private:
	MCallbackIdArray callbackIds;
};

#endif
