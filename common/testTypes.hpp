#ifndef TESTTYPES_H
#define TESTTYPES_H

#include <msgpack.hpp>

#include <unordered_map>

enum TestType { TEST, SCENE_UPDATE, SCENE_REQUEST };

MSGPACK_ADD_ENUM(TestType);

class TestClass
{
public:
	TestClass(int _id, TestType _req)
		:
		id(_id),
		type("Test"),
		reqType(_req)
	{
		attribs.insert({"Height", "50px"});
		attribs.insert({ "Width", "100px" });
	};

	TestClass()
		:
		id(0),
		type(""),
		reqType(TEST)
	{

	}

	unsigned int getID() { return id; }
	std::string getType() { return type; }
	TestType getRequestType() { return reqType; }
	auto getAttribs() { return attribs; }

private:
	unsigned int id;
	std::string type;
	TestType reqType;
	std::unordered_map<std::string, std::string> attribs;

public:
	MSGPACK_DEFINE(id, type, attribs, reqType);
};

#endif