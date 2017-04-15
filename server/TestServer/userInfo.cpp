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

void UserInfo::updateUser(std::string& _id)
{
	// this is last time the user made a request.
	userDB[_id] = std::time(nullptr);
}

void UserInfo::storeUsersToFile()
{
	std::ofstream outputFile("users.json");
	outputFile << std::setw(4) << userDB << std::endl;
	outputFile.close();
}
