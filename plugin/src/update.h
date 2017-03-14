#ifndef UPDATE_H
#define UPDATE_H

#include "polyModifierCmd.h"

#include "maya/MArgList.h"
#include "maya/MFnDependencyNode.h"
#include <maya/MSyntax.h>
#include <memory>

class Messaging;

class Update : public polyModifierCmd
{
public:
	Update();
	~Update();

	static void* creator();
	static MSyntax Update::newSyntax();
	MStatus	doIt(const MArgList&);

private:
	MStatus getArgs(const MArgList& args, MString& address, int& port);
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