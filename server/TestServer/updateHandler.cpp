#include "updateHandler.h"

#include "testTypes.h"
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

	std::vector<json> nodeList = currentMesh["nodes"];

	for (auto& newNode : _mesh["nodes"])
	{
		std::string id = newNode["id"];

		for (unsigned int i = 0; i < nodeList.size(); ++i)
		{
			if (id.compare(nodeList[i]["id"]) == 0)
			{
				if (newNode["edit"] == EditType::EDIT)
				{
					std::cout << "edit : " << id << std::endl;
					nodeList[i] = newNode;
				}
				else if (newNode["edit"] == EditType::DEL)
				{
					std::cout << "del : " << id << std::endl;
					nodeList.erase(nodeList.begin() + i);
				}
			}
		}
	}

	currentMesh["nodes"] = nodeList;;

	pDB->putMesh(currentMesh);
}
