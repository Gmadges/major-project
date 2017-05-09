#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <memory>
#include "json.h"

class Database;
class UserInfo;

class RequestHandler
{
public:
	RequestHandler(std::shared_ptr<Database> _db, std::shared_ptr<UserInfo> _user);
	~RequestHandler();

	json requestMesh(json& _request);
	json requestMeshUpdates(json& _request);

private:
	std::shared_ptr<Database> pDB;
	std::shared_ptr<UserInfo> pUserInfo;
};

#endif

