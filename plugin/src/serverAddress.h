#ifndef SERVERADDRESS_H
#define SERVERADDRESS_H

#include <string>

class ServerAddress
{
public:

	static ServerAddress& getInstance()
	{
		static ServerAddress instance;
		return instance;
	}

	// dont want these things happening
	ServerAddress(ServerAddress const&) = delete;
	void operator=(ServerAddress const&) = delete;

public:
	
	void setAddress(std::string _address);
	void setPort(int _port);

	std::string getAddress();
	int getPort();

	bool isServerSet();

private:
	// dont want people to be able to get this
	ServerAddress();
	~ServerAddress();

private:

	int port;
	std::string address;

	// this might be overkill
	bool bPortSet;
	bool bAddressSet;
};

#endif
