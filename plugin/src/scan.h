#ifndef SCAN_H
#define SCAN_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MPlug.h>

#include "genericMeshMessage.h"

#include <memory>

class Messaging;

class Scan : public MPxCommand
{
public:
	Scan();
	~Scan();

	static void* creator();
	virtual MStatus	doIt(const MArgList&);

private:
	void traverseHistory(MFnDependencyNode & node, std::vector<GenericNode>& nodeList);
	MStatus getGenericNode(MFnDependencyNode & _inNode, GenericNode & _outNode);
	MStatus getAttribFromPlug(MPlug& _plug, attribMap& _attribs);
	MStatus sendMesh(MDagPath & meshNode);

	std::unique_ptr<Messaging> pMessaging;
};

#endif