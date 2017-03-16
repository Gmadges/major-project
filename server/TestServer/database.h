#ifndef DATABASE_H
#define DATABASE_H

#include "json.h"

class Database
{
public:
	Database();
	~Database();

	bool putMesh(json& _mesh);
	json getMesh(std::string& _name);

private:
	json db;
};

#endif

