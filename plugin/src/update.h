#ifndef UPDATE_H
#define UPDATE_H

#include <memory>

class Subscriber;

class Update
{
public:
	Update();
	~Update();

private:
	std::unique_ptr<Subscriber> pSubscriber;
};

#endif