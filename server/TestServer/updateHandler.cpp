#include "updateHandler.h"


#include "database.h"
#include <thread>

UpdateHandler::UpdateHandler(std::shared_ptr<Database> _db)
	:
	pDB(_db)
{
}


UpdateHandler::~UpdateHandler()
{
}

bool UpdateHandler::registerMesh(json& _request)
{
	if (_request.count("mesh") > 0)
	{
		return pDB->putMesh(_request["mesh"]);
	}

	return false;
}

bool UpdateHandler::updateMesh(json& _request)
{
	if (_request.count("mesh") > 0)
	{
		// fire off to do comparison
		// do on another thread might be long
		std::thread job(&UpdateHandler::updateAndStoreMesh, this, _request["mesh"]);
		job.detach();

		return true;
	}

	return false;
}

void UpdateHandler::updateAndStoreMesh(json _mesh)
{
	// super right now
	// replaces any nodes with the new versions of them selves.

	json currentMesh = pDB->getMesh(_mesh["id"].get<std::string>());

	for (auto& newNode : _mesh["nodes"])
	{
		std::string id = newNode["id"];

		for (auto& curNode : currentMesh["nodes"])
		{
			if (id.compare(curNode["id"]) == 0)
			{
				curNode = newNode;
			}
		}
	}

	pDB->putMesh(currentMesh);
}
