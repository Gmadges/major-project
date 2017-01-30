#include "server.h"

#include <iostream>

Server::Server(zmq::context_t& _context)
	:
	recieveSocket(_context, ZMQ_REP)
{
	// client socket that recieves changes
	recieveSocket.bind("tcp://*:5555");
}


Server::~Server()
{
}

int Server::run()
{
	while (true) {
		zmq::message_t request;

		//  Wait for next request from client
		recieveSocket.recv(&request);
		std::cout << "Received Hello" << std::endl;

		//  Do some 'work'

		//  Send reply back to client
		zmq::message_t reply(5);
		memcpy(reply.data(), "World", 5);
		recieveSocket.send(reply);
	}
}
