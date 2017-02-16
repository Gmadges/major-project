#ifndef SCANMESSAGE_H
#define SCANMESSAGE_H

#include <msgpack.hpp>
#include <unordered_map>

enum RequestType { REQTEST, SCENE_UPDATE, SCENE_REQUEST };

MSGPACK_ADD_ENUM(RequestType);

// typedefs for easy
typedef std::pair<std::string, msgpack::object> attribType;
typedef std::unordered_map<std::string, msgpack::object> attribMap;

class GenericMessage
{
public:
	GenericMessage()
		:
		reqType(REQTEST)
	{
	}

	//setters
	void setName(std::string& _name)			{ name = _name; }
	void setRequestType(RequestType _type)		{ reqType = _type; }
	void setNodeType(std::string _type)			{ nodeType = _type; }
	void setAttribs(attribMap _attribs)			{ attribs = _attribs; }

	// getters
	std::string getName()			{ return name; }
	RequestType getRequestType()	{ return reqType; }
	std::string getNodeType()		{ return nodeType; }
	auto getAttribs()				{ return attribs; }

private:

	std::string name;
	std::string nodeType;
	RequestType reqType;
	attribMap attribs;

public:
	MSGPACK_DEFINE(name, nodeType, reqType, attribs);
};

#endif