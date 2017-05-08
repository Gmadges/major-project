#ifndef REQUESTABSTRACT_H
#define REQUESTABSTRACT_H

#include "maya/MArgList.h"
#include "maya/MFnDependencyNode.h"
#include <maya/MSyntax.h>
#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include <memory>

#include "json.h"

class TweakHandler;

class RequestAbstract : public MPxCommand
{
public:
	RequestAbstract();
	~RequestAbstract();
	static MSyntax newSyntax();

protected:
	MStatus getArgs(const MArgList& args, MString& id);
	MStatus setNodeValues(json & _node);
	MStatus createNode(json& _node);
	MStatus setConnections(json& _mesh, json& _node);
	MStatus setAttribs(MFnDependencyNode& node, json& attribs);
	MStatus disconnectNodes(MFnDependencyNode& inNode, MFnDependencyNode& outnode);

protected:
	std::unique_ptr<TweakHandler> pTweakHandler;
	MDGModifier fDGModifier;
};

#endif