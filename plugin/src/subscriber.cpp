#include "subscriber.h"

#include <iostream>

Subscriber::Subscriber(std::string _port)
	:
	updates(0),
	port(_port),
	context(1),
	socket(context, ZMQ_SUB),
	bListen(true)
	//listenThread(&Subscriber::listenForUpdates, this)
{
	// bind the subscriber
	socket.connect("tcp://localhost:"+port);
	
	// this is to filter out messages we dont want to see.
	// socket.setsockopt(ZMQ_SUBSCRIBE, "B");
}


Subscriber::~Subscriber()
{
	// this should hopefully end the loop
	bListen = false;
	listenThread.join();
}

void Subscriber::listenForUpdates()
{
	while (bListen)
	{
		zmq::message_t update;

		socket.recv(&update);

		std::cout << static_cast<char*>(update.data()) << std::endl;

		updates++;
	}
}
