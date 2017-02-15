#include "server.h"

#include <thread>
#include <iostream>
#include <msgpack.hpp>

#include "genericMessage.h"

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

		GenericMessage data;
		msgpack::object_handle oh = msgpack::unpack(static_cast<char *>(request.data()), request.size());
		oh.get().convert(data);

		// printing boi
		std::cout << "THREAD: " << std::this_thread::get_id() << std::endl;
		std::cout << "NAME: " << data.getName() << std::endl;
		std::cout << "NODE TYPE: " << data.getNodeType() << std::endl;

		auto attribs = data.getAttribs();

		std::cout << "ATTRIBS:" << std::endl;
		for (auto it : attribs)
		{
			std::cout << it.first << " : " << it.second << std::endl;
		}

		// Type
		switch (data.getRequestType())
		{
			case SCENE_UPDATE: 
			{
				std::cout << "we got an update!" << std::endl;
				std::cout << "add to stack!" << std::endl;

				// add to stack
				msgStack.push_back(data);

				//  Send reply back to client
				zmq::message_t reply(7);
				memcpy(reply.data(), "SUCCESS", 7);
				socket.send(reply);
				break;
			}
			case SCENE_REQUEST:
			{
				std::cout << "ugh someone wants our data!" << std::endl;

				if (msgStack.empty())
				{
					GenericMessage msg;

					msgpack::sbuffer sbuf;
					msgpack::pack(sbuf, msg);

					// send reply
					zmq::message_t reply(sbuf.size());
					std::memcpy(reply.data(), sbuf.data(), sbuf.size());
					socket.send(reply);
					break;
				}

				// get our current data
				// pack a message up
				msgpack::sbuffer sbuf;
				msgpack::pack(sbuf, msgStack.back());

				// send reply
				zmq::message_t reply(sbuf.size());
				std::memcpy(reply.data(), sbuf.data(), sbuf.size());
				socket.send(reply);

				msgStack.pop_back();
				break;
			}
		};
	}
}