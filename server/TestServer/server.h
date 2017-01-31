#ifndef SERVER_H
#define SERVER_H

#include <zmq.hpp>
#include <vector>

class Server
{
public:
	Server(zmq::context_t& _context);
	~Server();

	int run();
	void pushUpdate();

private:

	zmq::socket_t recieveSocket;
	zmq::socket_t updateSocket;
};

#endif
