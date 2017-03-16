#include "updateHandler.h"


#include "database.h"

UpdateHandler::UpdateHandler(std::shared_ptr<Database> _db)
	:
	pDB(_db)
{
}


UpdateHandler::~UpdateHandler()
{
}

bool UpdateHandler::processRequest(json& _request)
{
	if (_request.count("mesh") > 0)
	{
		return pDB->putMesh(_request["mesh"]);
	}

	return false;
}
