#ifndef DATASTORE_H
#define DATASTORE_H

#include <string>
#include <unordered_map>
#include <ctime>

class DataStore
{
public:

	static DataStore& getInstance()
	{
		static DataStore instance;
		return instance;
	}

	// dont want these things happening
	DataStore(DataStore const&) = delete;
	void operator=(DataStore const&) = delete;

public:

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

	bool anyChanges();

	std::string getCurrentRegisteredMesh();
	void setCurrentRegisteredMesh(std::string meshID);
	
	void setIPAddress(std::string _address);
	void setPort(int _port);
	void setUserID(std::string _id);

	std::string getAddress();
	std::string getUserID();
	int getPort();

	bool isServerSet();

	void setFullMeshRequest(bool _fullMesh);
	bool getFullMeshRequest();

private:	
	// dont want people to be able to get this
	DataStore();
	~DataStore();

private:
	std::unordered_map<std::string, std::time_t> editList;
	std::unordered_map<std::string, std::time_t> addList;
	std::unordered_map<std::string, std::time_t> delList;
	std::string currentMeshID;

	int port;
	std::string address;
	std::string userID;

	// this might be overkill
	bool bPortSet;
	bool bAddressSet;

	bool bFullMeshRequests;
};

#endif
