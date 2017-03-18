#ifndef UPDATE_H
#define UPDATE_H

#include "polyModifierCmd.h"

#include "maya/MArgList.h"
#include "maya/MFnDependencyNode.h"
#include <maya/MSyntax.h>
#include <memory>

#include "json.h"

class Messaging;
class TweakHandler;

class Update : public polyModifierCmd
{
public:
	Update();
	~Update();

	static void* creator();
	static MSyntax Update::newSyntax();
	MStatus	doIt(const MArgList&);

private:
	MStatus getArgs(const MArgList& args, MString& address, int& port, MString& id);
	MStatus setNodeValues(json & data);
	bool doesItExist(MString& name);
	MStatus createMesh(json& _mesh);
	void renameNodes(MFnDependencyNode & node, json& mesh);
	MStatus createNode(json& _node);
	MStatus setConnections(json& _mesh, json& _node);
	MStatus setAttribs(MFnDependencyNode& node, json& attribs);

private:
	std::unique_ptr<Messaging> pMessenger;
	std::unique_ptr<TweakHandler> pTweakHandler;
};

#endif