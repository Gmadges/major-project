#include "server.h"

#include <thread>
#include <iostream>
#include <msgpack.hpp>

#include "testTypes.hpp"

Server::Server()
	:
	context(1),
	recieveSocket(context, ZMQ_ROUTER),
	workersSocket(context, ZMQ_DEALER)
{
}


Server::~Server()
{
}

int Server::run()
{
	// client socket that recieves changes
	recieveSocket.bind("tcp://*:8080");
	workersSocket.bind("inproc://workers");

	unsigned int maxThreads = std::thread::hardware_concurrency();

	//  Launch pool of worker threads
	for (unsigned int i = 0; i < maxThreads; i++)
	{
		workers.push_back(std::thread(&Server::handleRequest, this));
	}

	//  Connect work threads to client threads via a queue
	zmq::proxy(recieveSocket, workersSocket, NULL);

	for (unsigned int i = 0; i < maxThreads; i++)
	{
		workers[i].join();
	}

	return 1;
}

void Server::handleRequest() 
{
	unsigned int count = 0;

	zmq::socket_t socket(context, ZMQ_REP);
	socket.connect("inproc://workers");

	while (true) {
		zmq::message_t request;

		//  Wait for next request from client
		socket.recv(&request);

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

		std::cout << "THREAD: " << std::this_thread::get_id() << std::endl;
		std::cout << "ID: " << data.getID() << std::endl;
		std::cout << "TYPE: " << data.getType() << std::endl;

		auto attribs = data.getAttribs();

		for (auto it : attribs)
		{
			std::cout << it.first << " : " << it.second << std::endl;
		}

		//  Do some 'work'
		count++;

		//  Send reply back to client
		zmq::message_t reply(5);
		memcpy(reply.data(), "World", 5);
		socket.send(reply);
	}
}

