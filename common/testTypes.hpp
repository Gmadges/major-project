#ifndef TESTTYPES_H
#define TESTTYPES_H

#include <msgpack.hpp>

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

#endif