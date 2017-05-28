#include "database.h"

#include <fstream>

Database::Database()
{
	std::ifstream dbfile("database.json");
	if (dbfile.is_open())
	{
		dbfile >> db;
	}
}

Database::~Database()
{
}

bool Database::putMesh(json& _mesh)
{
	// lock
	std::lock_guard<std::mutex> lock(mut);

	if (_mesh.count("id") > 0)
	{
		std::string id = _mesh["id"];
		std::cout << "storing: " << id << std::endl;
		
		_mesh["db_time"] = std::time(nullptr);

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
		if (db[_id].count("mesh") > 0)
		{
			return db[_id]["mesh"];
		}
	}

	return json();
}

json Database::getMeshWithEdits(std::string& _id)
{
	if (db.count(_id) > 0)
	{
		return db[_id];
	}

	return json();
}

std::vector<json> Database::getMeshEdits(std::string& _id)
{
	try
	{
		if (db.count(_id) > 0)
		{
			if (db[_id].count("edits") > 0)
			{
				return db[_id]["edits"].get<std::vector<json>>();
			}
		}
	}
	catch (std::exception& e)
	{
		std::cout << e.what();
	}

	return std::vector<json>();
}

bool Database::putMeshWithEdits(json& _object)
{
	// lock
	std::lock_guard<std::mutex> lock(mut);

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

bool Database::deleteMesh(std::string& id)
{
	if (db.count(id) > 0)
	{
		db.erase(id);
		storeDBToFile();
		return true;
	}
	return false;
}