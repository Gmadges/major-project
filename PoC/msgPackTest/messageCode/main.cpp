#include <zmq.hpp>
#include <msgpack.hpp>
#include <string>
#include <iostream>

class TestClass
{
public:
	TestClass(int _id, std::string _type)
		:
		id(_id),
		type(_type)
	{
	};

	TestClass()
		:
		id(0),
		type("")
	{

	}

	unsigned int getID() { return id; }
	std::string getType() { return type; }

private:
	unsigned int id;
	std::string type;

public:
	MSGPACK_DEFINE(id, type);
};

int main()
{
	//  Prepare our context and socket
	zmq::context_t context(1);
	zmq::socket_t socket(context, ZMQ_REQ);

	std::cout << "Connecting to hello world server…" << std::endl;
	socket.connect("tcp://localhost:8080");

	//  Do 10 requests, waiting each time for a response
	for (int request_nbr = 0; request_nbr != 10; request_nbr++) {
		
		// create my class
		TestClass test(request_nbr, "TEST");

		// pack a message up
		msgpack::sbuffer sbuf;
		msgpack::pack(sbuf, test);

		zmq::message_t request(sbuf.size());
		std::memcpy(request.data(), sbuf.data(), sbuf.size());
		
		std::cout << "Sending Hello " << request_nbr << "…" << std::endl;
		socket.send(request);

		//  Get the reply.
		zmq::message_t reply;
		socket.recv(&reply);
		std::cout << "Received World " << request_nbr << std::endl;
	}
	return 0;
}