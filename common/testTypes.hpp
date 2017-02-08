#ifndef TESTTYPES_H
#define TESTTYPES_H

#include <msgpack.hpp>

#include <unordered_map>

class TestClass
{
public:
	TestClass(int _id, std::string _type)
		:
		id(_id),
		type(_type)
	{
		attribs.insert({"Height", "50px"});
		attribs.insert({ "Width", "100px" });
	};

	TestClass()
		:
		id(0),
		type("")
	{

	}

	unsigned int getID() { return id; }
	std::string getType() { return type; }
	auto getAttribs() { return attribs; }

private:
	unsigned int id;
	std::string type;
	std::unordered_map<std::string, std::string> attribs;

public:
	MSGPACK_DEFINE(id, type, attribs);
};

#endif