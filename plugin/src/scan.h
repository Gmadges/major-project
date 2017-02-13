#ifndef SCAN_H
#define SCAN_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnDependencyNode.h>

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
	void findHistory(MFnDependencyNode& node);
	void sendPolySplitNode(MFnDependencyNode& node);

	std::unique_ptr<Messaging> pMessaging;
};

#endif