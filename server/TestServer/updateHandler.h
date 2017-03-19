#ifndef UPDATEHANDLER_H
#define UPDATEHANDLER_H

#include <memory>
#include "json.h"

class Database;

class UpdateHandler
{
public:
	UpdateHandler(std::shared_ptr<Database> _db);
	~UpdateHandler();

	bool processRequest(json& _request);

private:
	std::shared_ptr<Database> pDB;
};

#endif

