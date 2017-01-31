#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <string>
#include <thread>
#include <zmq.hpp>

class Subscriber
{
public:
	Subscriber(std::string _port);
	~Subscriber();

	void listenForUpdates();

public:
	// testing
	int updates;

private:

	std::string port;
	zmq::context_t context;
	zmq::socket_t socket;
	bool bListen;
	std::thread listenThread;
};

#endif

