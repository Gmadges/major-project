#ifndef UPDATE_H
#define UPDATE_H

#include "polyModifierCmd.h"

#include "maya/MArgList.h"
#include "maya/MFnDependencyNode.h"
#include <memory>

#include "genericMeshMessage.h"

class Messaging;

class Update : public polyModifierCmd
{
public:
	Update();
	~Update();

	static void* creator();
	MStatus	doIt(const MArgList&);

private:
	MStatus setNodeValues(GenericNode & data);
	bool doesItExist(MString& name);
	MStatus createMesh(GenericMesh& _mesh);
	void renameNodes(MFnDependencyNode & node, GenericMesh& mesh);
	MStatus createNode(GenericNode& _node);
	MStatus setConnections(GenericMesh& _mesh, GenericNode& _node);

private:
	std::unique_ptr<Messaging> pMessenger;

};

#endif