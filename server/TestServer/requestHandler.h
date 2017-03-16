#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <memory>
#include "json.h"

class Database;

class RequestHandler
{
public:
	RequestHandler(std::shared_ptr<Database> _db);
	~RequestHandler();

	json processRequest(json& _request);

private:
	std::shared_ptr<Database> pDB;
};

#endif

