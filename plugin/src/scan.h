#ifndef SCAN_H
#define SCAN_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MPlug.h>

#include "genericMessage.h"

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
	void traverseHistory(MFnDependencyNode& node, MFnMesh& mesh);
	void sendNode(MFnDependencyNode& node, MFnMesh& mesh);

	MStatus getAttribFromPlug(MPlug& _plug, attribMap& _attribs);

	std::unique_ptr<Messaging> pMessaging;
};

#endif