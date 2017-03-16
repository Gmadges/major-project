#include "requestHandler.h"

#include "database.h"

RequestHandler::RequestHandler(std::shared_ptr<Database> _db)
	:
	pDB(_db)
{
}


RequestHandler::~RequestHandler()
{
}

json RequestHandler::processRequest(json& _request)
{
	return json();
}
