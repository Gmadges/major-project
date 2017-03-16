#include "server.h"

#include <thread>
#include <iostream>
#include "testTypes.h"
#include "json.h"

#include "requestHandler.h"
#include "updateHandler.h"

// this is only for our fake one right now
#include "database.h"

Server::Server(int _port)
	:
	context(1),
	recieveSocket(context, ZMQ_ROUTER),
	workersSocket(context, ZMQ_DEALER),
	port(_port)
{
	std::shared_ptr<Database> pDB(new Database());
	pUpdateHandler.reset(new UpdateHandler(pDB));
	pRequestHandler.reset(new RequestHandler(pDB));
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
		workers.push_back(std::thread(&Server::handleMessage, this));
	}

	//  Connect work threads to client threads via a queue
	zmq::proxy(recieveSocket, workersSocket, NULL);

	for (unsigned int i = 0; i < maxThreads; i++)
	{
		workers[i].join();
	}

	return 1;
}

void Server::handleMessage() 
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

		// Type
		ReqType reqType = data["requestType"];
		switch (reqType)
		{
			case SCENE_UPDATE: 
			{
				bool result = pUpdateHandler->processRequest(data);

				json replyData;
				replyData["result"] = result;
				auto sendBuff = json::to_msgpack(replyData);
				zmq::message_t reply(sendBuff.size());
				std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
				socket.send(reply);
				break;
			}
			case SCENE_REQUEST:
			{
				json replyData = pRequestHandler->processRequest(data);
				auto sendBuff = json::to_msgpack(replyData);
				zmq::message_t reply(sendBuff.size());
				std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
				socket.send(reply);
				break;
			}
		};
	}
}