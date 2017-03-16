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
	if (_request.count("meshName") > 0)
	{
		return pDB->getMesh(_request["meshName"].get<std::string>());
	}

	return json();
}
