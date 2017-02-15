#ifndef MESSAGING_H
#define MESSAGING_H

#include <zmq.hpp>
#include "genericMessage.h"

#include <functional>

class Messaging
{
public:
	Messaging(std::string _port);
	~Messaging();
	
	bool sendUpdate(const GenericMessage& data);
	GenericMessage requestData();

private:
	bool pollForReply(std::function<void()> replyFunc, std::function<void()> sendFunc);
	void resetSocket();

private:
	std::string port;

	zmq::context_t context;
	zmq::socket_t socket;
};

#endif