#ifndef SERVER_H
#define SERVER_H

#include <zmq.hpp>
#include <vector>
#include <thread>

//test
#include "genericMessage.h"

class Server
{
public:
	Server();
	~Server();

	int run();

private:
	void handleRequest();

private:
	zmq::context_t context;
	zmq::socket_t recieveSocket;
	zmq::socket_t workersSocket;

	std::vector<std::thread> workers;

	//testing
	std::vector<GenericMessage> msgStack;
};

#endif
