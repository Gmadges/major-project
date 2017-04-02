#ifndef DATABASE_H
#define DATABASE_H

#include "json.h"

class Database
{
public:
	Database();
	~Database();

	bool putMesh(json& _mesh);
	bool putMeshWithEdits(json& _object);

	json getMesh(std::string& _id);
	json getMeshWithEdits(std::string& _id);

	// this method will change later, i'm sure of it
	json getAllMeshes();

private:
	void storeToFile();

private:
	json db;
};

#endif

