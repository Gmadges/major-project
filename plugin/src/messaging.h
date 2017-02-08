#ifndef MESSAGING_H
#define MESSAGING_H

#include <zmq.hpp>

class Messaging
{
public:
	Messaging(std::string _port);
	~Messaging();
	
	template<typename T>
	void send(const T& data);

private:
	std::string port;

	zmq::context_t context;
	zmq::socket_t socket;
};

#include <msgpack.hpp>

template<typename T>
void Messaging::send(const T& data)
{
	// pack a message up
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, data);

	zmq::message_t request(sbuf.size());
	std::memcpy(request.data(), sbuf.data(), sbuf.size());

	socket.send(request);

	zmq::message_t reply;
	socket.recv(&reply);
}

#endif