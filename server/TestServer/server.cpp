#include "server.h"

#include <thread>
#include <iostream>
#include "testTypes.h"
#include "json.h"

Server::Server(int _port)
	:
	context(1),
	recieveSocket(context, ZMQ_ROUTER),
	workersSocket(context, ZMQ_DEALER),
	port(_port)
{
}

Server::~Server()
{
}

int Server::run()
{
	// client socket that recieves changes
	recieveSocket.bind("tcp://*:"+std::to_string(port));
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

		uint8_t *uintBuf = (uint8_t*)request.data();
		std::vector<uint8_t> reqBuffer(uintBuf, uintBuf + request.size());
		json data = json::from_msgpack(reqBuffer);

		// printing boi
		std::cout << "THREAD: " << std::this_thread::get_id() << std::endl;
		
		std::cout << data.dump(4) << std::endl;

		// Type
		ReqType reqType = data["requestType"];
		switch (reqType)
		{
			case SCENE_UPDATE: 
			{
				std::cout << "we got an update!" << std::endl;
				std::cout << "add to stack!" << std::endl;

				// add to stack
				msgQueue.push(data);

				//  Send reply back to client
				zmq::message_t reply(7);
				memcpy(reply.data(), "SUCCESS", 7);
				socket.send(reply);
				break;
			}
			case SCENE_REQUEST:
			{
				std::cout << "ugh someone wants our data!" << std::endl;

				if (msgQueue.empty())
				{
					json empty;
					auto sendBuff = json::to_msgpack(empty);
					zmq::message_t reply(sendBuff.size());
					std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
					socket.send(reply);
					break;
				}

				auto sendBuff = json::to_msgpack(msgQueue.front());
				zmq::message_t reply(sendBuff.size());
				std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
				socket.send(reply);

				msgQueue.pop();
				break;
			}
		};
	}
}