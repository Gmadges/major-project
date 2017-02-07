#include "server.h"

#include <iostream>

Server::Server(zmq::context_t& _context)
	:
	recieveSocket(_context, ZMQ_REP),
	updateSocket(_context, ZMQ_PUB)
{
	// client socket that recieves changes
	recieveSocket.bind("tcp://*:8080");

	// publisher channel
	updateSocket.bind("tcp://*:9000");
}


Server::~Server()
{
}

int Server::run()
{
	unsigned int count = 0;

	while (true) {
		zmq::message_t request;

		//  Wait for next request from client
		recieveSocket.recv(&request);
		std::cout << "Received Hello " << count << std::endl;

		//  Do some 'work'
		count++;

		//  Send reply back to client
		zmq::message_t reply(5);
		memcpy(reply.data(), "World", 5);
		recieveSocket.send(reply);
	}
}


