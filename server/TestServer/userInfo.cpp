#include "userInfo.h"

#include <fstream>
#include <ctime>

UserInfo::UserInfo()
{
	std::ifstream userFile("users.json");
	if (userFile.is_open())
	{
		userFile >> userDB;
	}
}

UserInfo::~UserInfo()
{
}

void UserInfo::updateUser(std::string _id)
{
	// lock
	std::lock_guard<std::mutex> lock(mut);

	// this is last time the user made a request.
	userDB[_id] = std::time(nullptr);
	storeUsersToFile();
}

time_t UserInfo::getUsersLastUpdate(std::string _id)
{
	if (userDB.count(_id) > 0)
	{
		return userDB[_id].get<time_t>();
	}

	return 0;
}

void UserInfo::storeUsersToFile()
{
	std::ofstream outputFile("users.json");
	outputFile << std::setw(4) << userDB << std::endl;
	outputFile.close();
}
