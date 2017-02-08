#include "messaging.h"
#include <testTypes.hpp>

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

void Messaging::send()
{
	// create my class
	TestClass test(0, "TEST");

	// pack a message up
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, test);

	zmq::message_t request(sbuf.size());
	std::memcpy(request.data(), sbuf.data(), sbuf.size());

	socket.send(request);

	zmq::message_t reply;
	socket.recv(&reply);
}
