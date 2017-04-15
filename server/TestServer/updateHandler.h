#ifndef UPDATEHANDLER_H
#define UPDATEHANDLER_H

#include <memory>
#include "json.h"

class Database;

class UpdateHandler
{
public:
	UpdateHandler(std::shared_ptr<Database> _db);
	~UpdateHandler();

	bool registerMesh(json& _request);
	bool updateMesh(json& _request);

private:
	void updateAndStoreMesh(json _mesh, std::string userID);

	void removeNodefromList(json _node, std::vector<json> _nodeList);
	void editNodeInList(json _node, std::vector<json> _nodeList);
	void insertNodeIntoList(json _node, std::vector<json> _nodeList);

private:
	std::shared_ptr<Database> pDB;
};

#endif

