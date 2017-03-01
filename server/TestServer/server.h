#ifndef SERVER_H
#define SERVER_H

#include <zmq.hpp>
#include <queue>
#include <thread>

#include "genericMeshMessage.h"

class Server
{
public:
	Server(int _port);
	~Server();

	int run();

private:
	void handleRequest();

private:
	zmq::context_t context;
	zmq::socket_t recieveSocket;
	zmq::socket_t workersSocket;
	int port;

	std::vector<std::thread> workers;

	//testing
	std::queue<GenericMesh> msgQueue;
};

#endif
