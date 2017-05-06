#include "serverAddress.h"

ServerAddress::ServerAddress()
	:
	address(""),
	port(0),
	bPortSet(false),
	bAddressSet(false)
{
}

ServerAddress::~ServerAddress()
{
}

void ServerAddress::setAddress(std::string _address)
{
	address = _address;
	bAddressSet  = true;
}

void ServerAddress::setPort(int _port)
{
	port = _port;
	bPortSet = true;
}

std::string ServerAddress::getAddress()
{
	return address;
}

int ServerAddress::getPort()
{
	return port;
}

bool ServerAddress::isServerSet()
{
	return (bPortSet && bAddressSet);
}

void ServerAddress::setUserID(std::string _id)
{
	userID = _id;
}

std::string ServerAddress::getUserID()
{
	return userID;
}