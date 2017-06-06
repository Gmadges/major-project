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

protected:
	MStatus setNodeValues(json & _node);
	MStatus createNode(json& _node);
	MStatus setConnections(json& _node);
	MStatus setAttribs(MFnDependencyNode& node, json& attribs);
	MStatus setComponentListAttribute(std::vector<std::string> components, MPlug& _plug);
	MStatus setEdgeDescAttribute(std::vector<json> components, MPlug& _plug);
	MStatus setMatrixAttribute(std::vector<double> numbers, MPlug& _plug);

protected:
	MDGModifier fDGModifier;
	std::shared_ptr<TweakHandler> pTweakHandler;
};

#endif