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
	return false;
}
