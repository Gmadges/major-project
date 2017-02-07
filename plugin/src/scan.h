#ifndef SCAN_H
#define SCAN_H

#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnDependencyNode.h>

#include <memory>

class Messaging;

class Scan
{
public:
	Scan();
	~Scan();

	MStatus doScan();

private:

	void findHistory(MFnDependencyNode& node);
	void sendPolySplitNode(MFnDependencyNode& node);

	std::unique_ptr<Messaging> pMessaging;
};

#endif