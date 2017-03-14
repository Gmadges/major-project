#ifndef MESSAGING_H
#define MESSAGING_H

#include <zmq.hpp>
#include <functional>

#include "json.h"

class Messaging
{
public:
	Messaging(std::string _address, int _port);
	~Messaging();
	
	void resetSocket(std::string _address, int _port);
	bool sendUpdate(const json& data);
	bool requestData(json& data);

private:
	bool send(zmq::message_t& msg, zmq::message_t& reply);
	void resetSocket();

private:
	std::string ipAddress;
	int port;

	zmq::context_t context;
	zmq::socket_t socket;
};

#endif