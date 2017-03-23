#include "database.h"

#include <fstream>

Database::Database()
{
	std::ifstream inputFile("database.json");
	if (inputFile.is_open())
	{
		inputFile >> db;
	}
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

		storeToFile();
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

void Database::storeToFile()
{
	std::ofstream outputFile("database.json");
	outputFile << std::setw(4) << db << std::endl;
	outputFile.close();
}
