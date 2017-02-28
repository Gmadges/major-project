#ifndef GENERICNODEMESSAGE_H
#define GENERICNODEMESSAGE_H

#include <msgpack.hpp>
#include <unordered_map>

// typedefs for easy
typedef std::pair<std::string, msgpack::object> attribType;
typedef std::unordered_map<std::string, msgpack::object> attribMap;

class GenericNode
{
public:
	GenericNode()
	{
	}

	//setters
	void setNodeName(std::string& _name)		{ nodeName = _name; }
	void setNodeType(std::string _type)			{ nodeType = _type; }
	void setAttribs(attribMap _attribs)			{ attribs = _attribs; }

	// getters
	std::string getNodeName()		{ return nodeName; }
	std::string getNodeType()		{ return nodeType; }
	auto getAttribs()				{ return attribs; }

private:

	std::string nodeName;
	std::string nodeType;
	attribMap attribs;

public:
	MSGPACK_DEFINE(nodeName, nodeType, attribs);
};

#endif