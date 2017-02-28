#ifndef MESSAGING_H
#define MESSAGING_H

#include <zmq.hpp>
#include "genericMeshMessage.h"

#include <functional>

class Messaging
{
public:
	Messaging(std::string _port);
	~Messaging();
	
	bool sendUpdate(const GenericMesh& data);
	bool requestData(GenericMesh& data);

private:
	bool send(zmq::message_t& msg, zmq::message_t& reply);
	void resetSocket();

private:
	std::string port;

	zmq::context_t context;
	zmq::socket_t socket;
};

#endif