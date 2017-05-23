#ifndef DATABASE_H
#define DATABASE_H

#include "json.h"
#include <mutex>

class Database
{
public:
	Database();
	~Database();

	bool putMesh(json& _mesh);
	bool putMeshWithEdits(json& _object);

	bool deleteMesh(std::string& id);

	json getMesh(std::string& _id);
	json getMeshWithEdits(std::string& _id);
	std::vector<json> getMeshEdits(std::string& _id);

	// this method will change later, i'm sure of it
	json getAllMeshes();

private:
	void storeDBToFile();

private:
	std::mutex mut;
	json db;

};

#endif

