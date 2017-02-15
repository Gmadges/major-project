#include "messaging.h"
#include <msgpack.hpp>

Messaging::Messaging(std::string _port)
	:
	port(_port),
	context(1),
	socket(context, ZMQ_REQ)
{
	socket.connect("tcp://localhost:" + port);
}

Messaging::~Messaging()
{
}

bool Messaging::pollForReply(std::function<void()> replyFunc)
{
	//test hardcode
	int numTries = 1;
	int timeout = 2500; // miliseconds

	for (int i = 0; i < numTries; i++)
	{
		//  poll the socket and use specified timeout;
		zmq::pollitem_t items[] = { { socket, 0, ZMQ_POLLIN, 0 } };
		zmq::poll(&items[0], 1, timeout);

		// do we have a reponse?
		if (items[0].revents & ZMQ_POLLIN)
		{
			// we have a response to perform the required code;
			replyFunc();
			return true;
		}

		// no reply
		// lets try again maybe
		if (i == (numTries - 1))
		{
			// reset socket
			
			//  Old socket will be confused; close it and open a new one
			socket = zmq::socket_t(context, ZMQ_REQ);
			socket.connect("tcp://localhost:" + port);
			
			// TODO
			// send again using send lambda;
		}
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

	socket.send(request);

	// now we poll for a response

	// we need a reply function to handle our reply

	auto replyFunc = [this](){
		zmq::message_t reply;
		socket.recv(&reply); 
	};

	//TODO if we plan on retrying
	// we need a possible send function again?????
	
	// now lets try for a reply
	return pollForReply(replyFunc);
}

GenericMessage Messaging::requestData()
{
	GenericMessage msg;
	msg.setRequestType(SCENE_REQUEST);

	// pack a message up
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, msg);

	zmq::message_t request(sbuf.size());
	std::memcpy(request.data(), sbuf.data(), sbuf.size());

	socket.send(request);

	zmq::message_t reply;
	socket.recv(&reply);

	// unpack the data and return it
	GenericMessage data;
	msgpack::object_handle oh = msgpack::unpack(static_cast<char *>(reply.data()), reply.size());
	oh.get().convert(data);

	return data;
}
