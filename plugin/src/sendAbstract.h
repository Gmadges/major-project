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

class SendAbstract : public MPxCommand
{
public:
	SendAbstract();
	~SendAbstract();

	virtual MStatus	doIt(const MArgList&) = 0;

protected:

	// gets goes over all nodes for mesh and performs the specified func(including transform)
	void traverseAllValidNodesForMesh(MDagPath & dagPath, std::function<bool(MFnDependencyNode&)>& func);

	// goes over all nodes starting from one passed to it
	void traverseAllValidNodes(MFnDependencyNode & node, std::function<bool(MFnDependencyNode&)>& func);

	MStatus getGenericNode(MFnDependencyNode & _inNode, json& _outNode);
	MStatus getAttribFromPlug(MPlug& _plug, json& _attribs);
	MStatus getNumericDataFromAttrib(MPlug& _plug, json& _attribs);
	MStatus getTypeDataFromAttrib(MPlug& _plug, json& _attribs);
	MStatus getOtherDataFromAttrib(MPlug& _plug, json& _attribs);
	MStatus getUnitDataFromAttrib(MPlug& _plug, json& _attribs);
	std::vector<std::string> getCompList(MPlug& _plug, MStatus& status);

	MStatus getIncomingID(MFnDependencyNode & _inNode, MString& _id);
	MStatus getOutgoingID(MFnDependencyNode & _inNode, MString& _id);


	std::unique_ptr<Messaging> pMessaging;
};

#endif