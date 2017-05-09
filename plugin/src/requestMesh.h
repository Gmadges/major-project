#ifndef REQUESTMESH_H
#define REQUESTMESH_H

#include "requestAbstract.h"

#include "maya/MArgList.h"
#include "maya/MFnDependencyNode.h"
#include <maya/MSyntax.h>
#include <memory>

#include "json.h"

class Messaging;
class TweakHandler;

class RequestMesh : public RequestAbstract
{
public:
	RequestMesh();
	~RequestMesh();

	static void* creator();
	MStatus	doIt(const MArgList&);

private:

	MStatus createMesh(json& _mesh);
	void matchIDs(MFnDependencyNode & node, json& mesh);

private:
	std::unique_ptr<Messaging> pMessenger;
};

#endif