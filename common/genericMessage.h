#ifndef SCANMESSAGE_H
#define SCANMESSAGE_H

#include <msgpack.hpp>
#include <unordered_map>

enum RequestType { REQTEST, SCENE_UPDATE, SCENE_REQUEST };
enum NodeType { EMPTY, POLYSPLIT };

MSGPACK_ADD_ENUM(RequestType);
MSGPACK_ADD_ENUM(NodeType);

class GenericMessage
{
public:
	GenericMessage()
		:
		name(""),
		nodeType(EMPTY),
		reqType(REQTEST)
	{
	}

	//setters
	void setName(std::string& _name)										{ name = _name; }
	void setRequestType(RequestType _type)									{ reqType = _type; }
	void setNodeType(NodeType _type)										{ nodeType = _type; }
	void setAttribs(std::unordered_map<std::string, std::string> _attribs)	{ attribs = _attribs; }

	// getters
	std::string getName()			{ return name; }
	RequestType getRequestType()	{ return reqType; }
	NodeType getNodeType()			{ return nodeType; }
	auto getAttribs()				{ return attribs; }

private:

	std::string name;
	NodeType nodeType;
	RequestType reqType;
	std::unordered_map<std::string, std::string> attribs;

public:
	MSGPACK_DEFINE(name, nodeType, reqType, attribs);
};

#endif