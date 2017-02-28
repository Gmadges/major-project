#include "messaging.h"
#include <msgpack.hpp>

Messaging::Messaging(std::string _address, int _port)
	:
	ipAddress(_address),
	port(_port),
	context(1),
	socket(context, ZMQ_REQ)
{
	//socket.connect("tcp://" + ipAddress + ":" + std::to_string(port));
	//int linger = 0;
	//socket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
}

Messaging::~Messaging()
{
}

void Messaging::resetSocket(std::string _address, int _port)
{
	ipAddress = _address;
	port = _port;
	resetSocket();
}

void Messaging::resetSocket()
{
	socket = zmq::socket_t(context, ZMQ_REQ);
	socket.connect("tcp://" + ipAddress + ":" + std::to_string(port));

	int linger = 0;
	socket.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
}

bool Messaging::send(zmq::message_t& msg, zmq::message_t& reply)
{
	//test hardcode
	int numTries = 1;
	int timeout = 1000; // miliseconds

	// send message
	socket.send(msg);

	for (int i = 0; i < numTries; i++)
	{
		//  poll the socket and use specified timeout;
		zmq::pollitem_t items[] = { { socket, 0, ZMQ_POLLIN, 0 } };
		zmq::poll(&items[0], 1, timeout);

		// do we have a reponse?
		if (items[0].revents & ZMQ_POLLIN)
		{
			// we have a response to perform the required code;
			socket.recv(&reply);
			return true;
		}

		// no reply
		// lets try again maybe

		// reset socket
		resetSocket();
		
		//if (i < (numTries - 1))
		//{
		//	// send again using send lambda
		//	
		//	// Not gonna use do repeat tries for now
		//	//sendFunc()
		//}
	}

	// we couldnt connect return false
	return false;
}

bool Messaging::sendUpdate(const GenericMessage& data)
{
	// pack a message up
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, data);

	zmq::message_t request(sbuf.size());
	std::memcpy(request.data(), sbuf.data(), sbuf.size());

	zmq::message_t reply;

	// return this value because i dont do anything with the reply yet.
	return send(request, reply);
}

bool Messaging::requestData(GenericMessage& data)
{
	GenericMessage msg;
	msg.setRequestType(SCENE_REQUEST);

	// pack a message up
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, msg);

	zmq::message_t request(sbuf.size());
	std::memcpy(request.data(), sbuf.data(), sbuf.size());
	
	zmq::message_t reply;
	if (send(request, reply))
	{
		// unpack the data and return it
		msgpack::object_handle oh = msgpack::unpack(static_cast<char *>(reply.data()), reply.size());
		oh.get().convert(data);
		return true;
	}

	return false;
}
