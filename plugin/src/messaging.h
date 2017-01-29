#ifndef MESSAGING_H
#define MESSAGING_H

#include <zmq.hpp>

class Messaging
{
public:
	Messaging(int _port);
	~Messaging();

	void send();

private:
	int port;

	zmq::context_t context;
	zmq::socket_t socket;
};

#endif