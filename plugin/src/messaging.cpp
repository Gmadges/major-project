#include "messaging.h"

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
