#ifndef SENDABSTRACT_H
#define SENDABSTRACT_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>
#include <maya/MSyntax.h>
#include <maya/MDagPath.h>

#include <algorithm>
#include <memory>
#include "json.h"

class Messaging;
class TweakHandler;

class SendAbstract : public MPxCommand
{
public:
	SendAbstract();
	~SendAbstract();

	virtual MStatus	doIt(const MArgList&) = 0;

protected:

	// gets goes over all nodes for mesh and performs the specified func(including transform)
	void traverseAllValidNodesForMesh(MDagPath & dagPath, std::function<void(MFnDependencyNode&)>& func);

	// goes over all nodes starting from one passed to it
	void traverseAllValidNodes(MFnDependencyNode & node, std::function<void(MFnDependencyNode&)>& func);

	MStatus getGenericNode(MFnDependencyNode & _inNode, json& _outNode);
	MStatus getAttribFromPlug(MPlug& _plug, json& _attribs);

	std::unique_ptr<Messaging> pMessaging;
	std::unique_ptr<TweakHandler> pTweaksHandler;
};

#endif