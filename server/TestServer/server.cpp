#include "server.h"

#include <iostream>
#include <msgpack.hpp>

#include "testTypes.hpp"

Server::Server(zmq::context_t& _context)
	:
	recieveSocket(_context, ZMQ_REP)
{
	// client socket that recieves changes
	recieveSocket.bind("tcp://*:8080");
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

		// lets find out what this message is about

		// TODO
		
		// 3 basic requests we can expect

		// new scene data to store

		// request the current version of the scene

		// get available scenes to work with

		// testing msgpack recieve

		TestClass data;
		msgpack::object_handle oh = msgpack::unpack(static_cast<char *>(request.data()), request.size());
		oh.get().convert(data);

		std::cout << "ID: " << data.getID() << std::endl;
		std::cout << "TYPE: " << data.getType() << std::endl;

		//  Do some 'work'
		count++;

		//  Send reply back to client
		zmq::message_t reply(5);
		memcpy(reply.data(), "World", 5);
		recieveSocket.send(reply);
	}
}


