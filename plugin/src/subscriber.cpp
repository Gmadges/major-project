#include "subscriber.h"



Subscriber::Subscriber(std::string _port)
	:
	port(_port),
	context(1),
	socket(context, ZMQ_SUB)
{
	// bind the subscriber
	socket.connect("tcp://localhost:"+port);
	
	// this is to filter out messages we dont want to see.
	// socket.setsockopt(ZMQ_SUBSCRIBE, "B");
}


Subscriber::~Subscriber()
{
}
