#include "dataStore.h"

DataStore::DataStore()
	:
	address(""),
	port(0),
	bPortSet(false),
	bAddressSet(false),
	bFullMeshRequests(false),
	timeInterval(2.0)
{
}

DataStore::~DataStore()
{
}

void DataStore::setIPAddress(std::string _address)
{
	address = _address;
	bAddressSet  = true;
}

void DataStore::setPort(int _port)
{
	port = _port;
	bPortSet = true;
}

std::string DataStore::getAddress()
{
	return address;
}

int DataStore::getPort()
{
	return port;
}

bool DataStore::isServerSet()
{
	return (bPortSet && bAddressSet);
}

void DataStore::setUserID(std::string _id)
{
	userID = _id;
}

std::string DataStore::getUserID()
{
	return userID;
}

// deletes
std::unordered_map<std::string, std::time_t> DataStore::getDeletedList()
{
	return delList;
}

void DataStore::addNodeToDeleteList(std::string uuid, time_t time)
{
	delList[uuid] = time;
}

void DataStore::resetDeleteList()
{
	delList.clear();
}

// adds
std::unordered_map<std::string, std::time_t> DataStore::getAddedList()
{
	return addList;
}

void DataStore::addNodeToAddedList(std::string uuid, time_t time)
{
	addList[uuid] = time;
}

void DataStore::resetAddedList()
{
	addList.clear();
}

// edits
std::unordered_map<std::string, std::time_t> DataStore::getEditsList()
{
	return editList;
}

void DataStore::addNodeToEditList(std::string uuid, time_t time)
{
	editList[uuid] = time;
}

void DataStore::resetEditList()
{
	editList.clear();
}

std::string DataStore::getCurrentRegisteredMesh()
{
	return currentMeshID;
}

void DataStore::setCurrentRegisteredMesh(std::string meshID)
{
	currentMeshID = meshID;
}

bool DataStore::anyChanges()
{
	return (!addList.empty() ||
		!editList.empty() ||
		!delList.empty());
}

void DataStore::setFullMeshRequest(bool _fullMesh)
{
	bFullMeshRequests = _fullMesh;
}

bool DataStore::getFullMeshRequest()
{
	return bFullMeshRequests;
}

void DataStore::setUpdateInterval(double _timeInt)
{
	timeInterval = _timeInt;
}

double DataStore::getUpdateInterval()
{
	return timeInterval;
}