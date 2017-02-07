#include "subscriber.h"

#include <iostream>

Subscriber::Subscriber(std::string _port)
	:
	port(_port),
	context(1),
	socket(context, ZMQ_SUB)
{
	// bind the subscriber
	socket.connect("tcp://localhost:"+port);
}


Subscriber::~Subscriber()
{
}

void Subscriber::askForUpdates()
{

}
