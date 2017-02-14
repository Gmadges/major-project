#ifndef MESSAGING_H
#define MESSAGING_H

#include <zmq.hpp>
#include "genericMessage.h"

class Messaging
{
public:
	Messaging(std::string _port);
	~Messaging();
	
	void sendUpdate(const GenericMessage& data);
	GenericMessage requestData();

private:
	std::string port;

	zmq::context_t context;
	zmq::socket_t socket;
};

#endif