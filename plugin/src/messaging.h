#ifndef MESSAGING_H
#define MESSAGING_H

class Messaging
{
public:
	Messaging(int _port);
	~Messaging();

	void send();

private:
	int port;

};

#endif