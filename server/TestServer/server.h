#ifndef SERVER_H
#define SERVER_H

#include <zmq.hpp>
#include <queue>
#include <thread>

#include "json.h"

class Server
{
public:
	Server(int _port);
	~Server();

	int run();

private:
	void handleMessage();

private:
	zmq::context_t context;
	zmq::socket_t recieveSocket;
	zmq::socket_t workersSocket;
	int port;

	std::vector<std::thread> workers;

	//meshes
	std::queue<json> msgQueue;
};

#endif
