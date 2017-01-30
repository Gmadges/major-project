#include "subscriber.h"



Subscriber::Subscriber(std::string _port)
	:
	port(_port),
	context(1),
	socket(context, ZMQ_SUB)
{
	// bind the subscriber
}


Subscriber::~Subscriber()
{
}
