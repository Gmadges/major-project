#include "database.h"

Database::Database()
{
}


Database::~Database()
{
}


bool Database::putMesh(json& _mesh)
{
	if (_mesh.count("name") > 0)
	{
		std::string name = _mesh["name"];
		db[name] = _mesh;
		return true;
	}

	return false;
}

json Database::getMesh(std::string& _name)
{
	if (db.count(_name) > 0)
	{
		return db[_name];
	}

	return json();
}
