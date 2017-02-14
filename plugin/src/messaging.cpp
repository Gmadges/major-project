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

void Messaging::sendUpdate(const GenericMessage& data)
{
	// pack a message up
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, data);

	zmq::message_t request(sbuf.size());
	std::memcpy(request.data(), sbuf.data(), sbuf.size());

	socket.send(request);

	zmq::message_t reply;
	socket.recv(&reply);
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
