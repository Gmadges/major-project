#ifndef INFOHANDLER_H
#define INFOHANDLER_H

#include <memory>
#include "json.h"

class Database;

class InfoHandler
{
public:
	InfoHandler(std::shared_ptr<Database> _db);
	~InfoHandler();

	json processRequest();

private:
	std::shared_ptr<Database> pDB;
};

#endif

