#include <zmq.hpp>
#include <msgpack.hpp>
#include <string>
#include <iostream>

#include "testTypes.hpp"

int main()
{
	//  Prepare our context and socket
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);

	std::cout << "Connecting to hello world server…" << std::endl;
	socket.connect("tcp://localhost:8080");

	//  Do 10 requests, waiting each time for a response
	for (int i = 0; i < 10; i++) {
		
		// create my class
		TestClass test(i, "TEST");

		// pack a message up
		msgpack::sbuffer sbuf;
		msgpack::pack(sbuf, test);

		zmq::message_t request(sbuf.size());
		std::memcpy(request.data(), sbuf.data(), sbuf.size());
		
		std::cout << "Sending Hello " << i << "…" << std::endl;
		socket.send(request);

		//  Get the reply.
		zmq::message_t reply;
		socket.recv(&reply);
		std::cout << "Received World " << i << std::endl;
	}
	return 0;
}