#include "database.h"

Database::Database()
{
}


Database::~Database()
{
}


bool Database::putMesh(json& _mesh)
{
	if (_mesh.count("id") > 0)
	{
		std::string id = _mesh["id"];
		std::cout << "storing: " << id << std::endl;
		db[id] = _mesh;
		return true;
	}

	return false;
}

json Database::getMesh(std::string& _id)
{
	if (db.count(_id) > 0)
	{
		std::cout << "getting: " << _id << std::endl;
		return db[_id];
	}

	return json();
}

json Database::getAllMeshes()
{
	return db;
}
