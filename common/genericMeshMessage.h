#ifndef GENERICMESHMESSAGE_H
#define	GENERICMESHMESSAGE_H

#include <msgpack.hpp>
#include <unordered_map>

#include "genericNodeMessage.h"

enum RequestType { REQTEST, SCENE_UPDATE, SCENE_REQUEST };
MSGPACK_ADD_ENUM(RequestType);

enum MeshType { EMPTY, CUBE, SPHERE };
MSGPACK_ADD_ENUM(MeshType);

class GenericMesh
{
public:
	GenericMesh()
		:
		reqType(REQTEST),
		meshType(EMPTY)
	{
	}

	//setters
	void setMeshName(std::string& _name) { meshName = _name; }
	void setMeshType(MeshType _type) { meshType = _type; }
	void setRequestType(RequestType _type) { reqType = _type; }
	void setNodes(std::vector<GenericNode> _nodes) { nodes = _nodes; }

	// getters
	std::string getMeshName() { return meshName; }
	MeshType getMeshType() { return meshType; }
	RequestType getRequestType() { return reqType; }
	auto getNodes() { return nodes; }

private:

	std::string meshName;
	MeshType meshType;
	RequestType reqType;

	// for now hope that heirachy order is correct
	std::vector<GenericNode> nodes;

public:
	MSGPACK_DEFINE(meshName, meshType, reqType, nodes);
};

#endif