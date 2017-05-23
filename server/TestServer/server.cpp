#include "server.h"

#include <thread>
#include <iostream>
#include "testTypes.h"
#include "json.h"

#include "requestHandler.h"
#include "updateHandler.h"
#include "userInfo.h"

// this is only for our fake one right now
#include "database.h"

#pragma warning(push, 0)
#include "crow.h"
#pragma warning(pop)

Server::Server(int _port)
	:
	context(1),
	recieveSocket(context, ZMQ_ROUTER),
	workersSocket(context, ZMQ_DEALER),
	port(_port),
	pDB(new Database()),
	pUserInfo(new UserInfo())
{
	pUpdateHandler.reset(new UpdateHandler(pDB));
	pRequestHandler.reset(new RequestHandler(pDB, pUserInfo));
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

	reqServerThread = std::thread(&Server::runDataServer, this);

	//  Connect work threads to client threads via a queue
	zmq::proxy(recieveSocket, workersSocket, NULL);

	for (unsigned int i = 0; i < maxThreads; i++)
	{
		workers[i].join();
	}

	reqServerThread.join();

	return 1;
}

void Server::runDataServer()
{
	crow::SimpleApp app;

	CROW_ROUTE(app, "/")([this]() {
		crow::json::wvalue info;
	
		info["status"] = 200;
		std::vector<std::string> meshNames;
		std::vector<std::string> meshIds;

		for (auto& item : pDB->getAllMeshes())
		{
			json mesh = item["mesh"];
			meshNames.push_back(mesh["name"].get<std::string>());
			meshIds.push_back(mesh["id"].get<std::string>());
		}
		
		info["meshNames"] = meshNames;
		info["meshIds"] = meshIds;
		
		return info;
	});

	CROW_ROUTE(app, "/heartbeat")([this]() {
		crow::json::wvalue info;
		info["status"] = 200;
		return info;
	});

	CROW_ROUTE(app, "/<str>/delete")([this](std::string id) {
		crow::json::wvalue info;

		if (pDB->deleteMesh(id))
		{
			info["status"] = 200;
			return info;
		}
		
		info["status"] = 404;
		return info;
	});

	app.port(port + 1).multithreaded().run();
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
				json replyData;

				try
				{
					bool result = pUpdateHandler->registerMesh(data);

					if (result)
					{
						pUserInfo->updateUser(data["uid"].get<std::string>());
					}

					replyData["result"] = result;
				}
				catch(std::exception& e)
				{
					std::cout << e.what() << std::endl;
					replyData["result"] = false;
				}

				auto sendBuff = json::to_msgpack(replyData);
				zmq::message_t reply(sendBuff.size());
				std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
				socket.send(reply);
				break;
			}
			case MESH_UPDATE:
			{
				json replyData;

				try
				{
					replyData["result"] = pUpdateHandler->updateMesh(data);
				}
				catch (std::exception& e)
				{
					std::cout << e.what() << std::endl;
					replyData["result"] = false;
				}

				auto sendBuff = json::to_msgpack(replyData);
				zmq::message_t reply(sendBuff.size());
				std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
				socket.send(reply);
				break;
			}
			case REQUEST_MESH:
			{
				json replyData;

				try
				{
					replyData = pRequestHandler->requestMesh(data);

					if (!replyData.empty())
					{
						pUserInfo->updateUser(data["uid"].get<std::string>());
					}
				}
				catch (std::exception& e)
				{
					std::cout << e.what() << std::endl;
				}

				auto sendBuff = json::to_msgpack(replyData);
				zmq::message_t reply(sendBuff.size());
				std::memcpy(reply.data(), sendBuff.data(), sendBuff.size());
				socket.send(reply);
				break;
			}
			case REQUEST_MESH_UPDATE:
			{
				json replyData;

				try
				{
					replyData = pRequestHandler->requestMeshUpdates(data);

					if (!replyData.empty())
					{
						if (!replyData["edits"].empty())
						{
							pUserInfo->updateUser(data["uid"].get<std::string>());
						}
					}
				}
				catch (std::exception& e)
				{
					std::cout << e.what() << std::endl;
				}

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