#include "infoHandler.h"

#include "database.h"


InfoHandler::InfoHandler(std::shared_ptr<Database> _db)
	:
	pDB(_db)
{
}


InfoHandler::~InfoHandler()
{
}

json InfoHandler::processRequest()
{
	// grab da info from database

	json info;

	for (auto& item : pDB->getAllMeshes())
	{
		std::string name = item["name"].get<std::string>();
		std::string id = item["id"].get<std::string>();

		info[id] = name;
	}

	return info;
}
