#ifndef DATABASE_H
#define DATABASE_H

#include "json.h"

class Database
{
public:
	Database();
	~Database();

	bool putMesh(json& _mesh);
	json getMesh(std::string& _id);

	// this method will change later, i'm sure of it
	json getAllMeshes();

private:
	void storeToFile();

private:
	json db;
};

#endif

