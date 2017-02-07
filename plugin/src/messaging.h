#ifndef MESSAGING_H
#define MESSAGING_H

#include <zmq.hpp>

class Messaging
{
public:
	Messaging(std::string _port);
	~Messaging();

	void send();

private:
	std::string port;

	zmq::context_t context;
	zmq::socket_t socket;
};

#endif