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

	static CallbackCreator& getInstance()
	{
		static CallbackCreator instance;
		return instance;
	}

	CallbackCreator(CallbackCreator const&) = delete;
	void operator=(CallbackCreator const&) = delete;

	// registers a callback to look at attribute changes
	// and another for its deletion
	MStatus registerCallbacksToNode(MObject& _node);
	MStatus registerCallbacksToDetectNewNodes();
	void removeCallbacks();

private:
	CallbackCreator() {};
	~CallbackCreator();

	//callback methods
	//void nodeChangeCallback(MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void*);

public:
	MCallbackIdArray callbackIds;
};

#endif
