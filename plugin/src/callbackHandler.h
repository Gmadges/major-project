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
	
	// timer callback
	MStatus startTimerCallback();

	// registering callbacks
	MStatus registerCallbacksToNode(MObject& _node);
	MStatus registerCallbacksToDetectNewNodes();

	std::string getCurrentRegisteredMesh();
	void setCurrentRegisteredMesh(std::string meshID);

	// deletes
	std::unordered_map<std::string, std::time_t> getDeletedList();
	void addNodeToDeleteList(std::string uuid, time_t time);
	void resetDeleteList();

	// adds
	std::unordered_map<std::string, std::time_t> getAddedList();
	void addNodeToAddedList(std::string uuid, time_t time);
	void resetAddedList();

	// edits
	std::unordered_map<std::string, std::time_t> getEditsList();
	void addNodeToEditList(std::string uuid, time_t time);
	void resetEditList();

private:
	// dont want people to be able to get this
	CallbackHandler() {};
	~CallbackHandler();

private:
	MCallbackIdArray callbackIds;

	std::unordered_map<std::string, std::time_t> editList;
	std::unordered_map<std::string, std::time_t> addList;
	std::unordered_map<std::string, std::time_t> delList;

	std::string currentMeshID;
};

#endif
