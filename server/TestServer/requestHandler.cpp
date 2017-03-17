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
	if (_request.count("id") > 0)
	{
		return pDB->getMesh(_request["id"].get<std::string>());
	}

	return json();
}
