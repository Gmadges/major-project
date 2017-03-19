#ifndef SERVER_H
#define SERVER_H

#include <zmq.hpp>
#include <thread>
#include <memory>

#include "json.h"

class RequestHandler;
class UpdateHandler;
class InfoHandler;

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

	std::unique_ptr<UpdateHandler> pUpdateHandler;
	std::unique_ptr<RequestHandler> pRequestHandler;
	std::unique_ptr<InfoHandler> pInfoHandler;
};

#endif
