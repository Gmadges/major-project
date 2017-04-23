#include "database.h"

#include <fstream>

Database::Database()
{
	std::ifstream dbfile("database.json");
	if (dbfile.is_open())
	{
		dbfile >> db;
	}

	std::ifstream userFile("users.json");
	if (userFile.is_open())
	{
		userFile >> userDB;
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

		storeDBToFile();
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

std::vector<json> Database::getMeshEdits(std::string& _id)
{
	if (db.count(_id) > 0)
	{
		std::cout << "getting: " << _id << std::endl;
		return db[_id]["edits"];
	}

	return std::vector<json>();
}

bool Database::putMeshWithEdits(json& _object)
{
	if (_object["mesh"].count("id") > 0)
	{
		std::string id = _object["mesh"]["id"];
		std::cout << "storing: " << id << std::endl;

		// lets timestamp the last entry to the db
		_object["edits"].back()["db_time"] = std::time(nullptr);

		db[id] = _object;

		storeDBToFile();
		return true;
	}

	return false;
}

json Database::getAllMeshes()
{
	return db;
}

void Database::storeDBToFile()
{
	std::ofstream outputFile("database.json");
	outputFile << std::setw(4) << db << std::endl;
	outputFile.close();
}

void Database::storeUsersToFile()
{
	std::ofstream outputFile("users.json");
	outputFile << std::setw(4) << userDB << std::endl;
	outputFile.close();
}
