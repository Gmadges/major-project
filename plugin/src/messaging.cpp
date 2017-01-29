#include "messaging.h"

Messaging::Messaging(int _port)
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
	zmq::message_t request(5);
	memcpy(request.data(), "Hello", 5);
	socket.send(request);
}
