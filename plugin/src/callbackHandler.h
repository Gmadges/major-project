#ifndef CALLBACKHANDLER_H
#define CALLBACKHANDLER_H

#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MCallbackIdArray.h>
#include <maya/MNodeMessage.h>
#include <maya/MPlug.h>

#include <unordered_map>
#include <ctime>

class CallbackHandler
{
public:

	static CallbackHandler& getInstance()
	{
		static CallbackHandler instance;
		return instance;
	}

	// dont want these things happening
	CallbackHandler(CallbackHandler const&) = delete;
	void operator=(CallbackHandler const&) = delete;

public:

	MStatus clearCallbacks();
	
	// timer callback
	MStatus startTimerCallback();

	// registering callbacks
	MStatus registerCallbacksToNode(MObject& _node);
	MStatus registerCallbacksToDetectNewNodes();

	void setIgnoreChanges(bool ignore);
	bool ignoreChanges();

private:
	// dont want people to be able to get this
	CallbackHandler();
	~CallbackHandler();

	MStatus cleanNodeOfCallbacks(MObject& _node);

private:
	MCallbackIdArray callbackIds;

	bool timerCallbackEnabled;
	bool newNodeCallbackEnabled;
	bool bIgnoreChanges;
};

#endif
