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

		json object;
		object["mesh"] = _mesh;
		object["edits"] = std::vector<json>();

		db[id] = object;

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
		return db[_id]["mesh"];
	}

	return json();
}

json Database::getMeshWithEdits(std::string& _id)
{
	if (db.count(_id) > 0)
	{
		std::cout << "getting: " << _id << std::endl;
		return db[_id];
	}

	return json();
}

bool Database::putMeshWithEdits(json& _object)
{
	if (_object["mesh"].count("id") > 0)
	{
		std::string id = _object["mesh"]["id"];
		std::cout << "storing: " << id << std::endl;

		db[id] = _object;

		storeToFile();
		return true;
	}

	return false;
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

bool Database::deleteMesh(std::string& id)
{
	if (db.count(id) > 0)
	{
		db.erase(id);
		return true;
	}
	return false;
}
