#include "server.h"

#include <thread>
#include <iostream>
#include "testTypes.h"
#include "json.h"

#include "requestHandler.h"
#include "updateHandler.h"
#include "infoHandler.h"

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
	pInfoHandler.reset(new InfoHandler(pDB));
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
			case REGISTER_MESH: 
			{
				bool result = pUpdateHandler->registerMesh(data);

				json replyData;
				replyData["result"] = result;
				auto sendBuff = json::to_msgpack(replyData);
				zmq::message_t reply(sendBuff.size());
				std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
				socket.send(reply);
				break;
			}
			case MESH_UPDATE:
			{
				bool result = pUpdateHandler->updateMesh(data);

				json replyData;
				replyData["result"] = result;
				auto sendBuff = json::to_msgpack(replyData);
				zmq::message_t reply(sendBuff.size());
				std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
				socket.send(reply);
				break;
			}
			case REQUEST_MESH:
			{
				json replyData = pRequestHandler->requestMesh(data);
				auto sendBuff = json::to_msgpack(replyData);
				zmq::message_t reply(sendBuff.size());
				std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
				socket.send(reply);
				break;
			}
			case REQUEST_MESH_UPDATE:
			{
				//TODO
				json replyData = pRequestHandler->requestMeshUpdates(data);
				break;
			}
			case INFO_REQUEST:
			{
				json replyData = pInfoHandler->processRequest();
				auto sendBuff = json::to_msgpack(replyData);
				zmq::message_t reply(sendBuff.size());
				std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
				socket.send(reply);
				break;
			}
			default : 
			{
				std::cout << "got a weird request!" << std::endl;
				zmq::message_t reply;
				socket.send(reply);
			}
		};
	}
}