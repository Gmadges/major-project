#ifndef USERINFO_H
#define USERINFO_H

#include "json.h"
#include <mutex>

class UserInfo
{
public:
	UserInfo();
	~UserInfo();

	void updateUser(std::string _id);
	time_t getUsersLastUpdate(std::string _id);

private:
	void storeUsersToFile();

private:
	std::mutex mut;
	json userDB;
};

#endif

