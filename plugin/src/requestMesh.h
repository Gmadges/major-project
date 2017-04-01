#ifndef REQUESTMESH_H
#define REQUESTMESH_H

#include "polyModifierCmd.h"

#include "maya/MArgList.h"
#include "maya/MFnDependencyNode.h"
#include <maya/MSyntax.h>
#include <memory>

#include "json.h"

class Messaging;
class TweakHandler;

class RequestMesh : public polyModifierCmd
{
public:
	RequestMesh();
	~RequestMesh();

	static void* creator();
	static MSyntax newSyntax();
	MStatus	doIt(const MArgList&);

private:
	MStatus getArgs(const MArgList& args, MString& id);
	MStatus setNodeValues(json & _node);
	bool doesItExist(std::string& id);
	MStatus createMesh(json& _mesh);
	void matchIDs(MFnDependencyNode & node, json& mesh);
	MStatus createNode(json& _node);
	MStatus setConnections(json& _mesh, json& _node);
	MStatus setAttribs(MFnDependencyNode& node, json& attribs);

private:
	std::unique_ptr<Messaging> pMessenger;
	std::unique_ptr<TweakHandler> pTweakHandler;
};

#endif