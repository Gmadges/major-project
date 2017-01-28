#ifndef SCAN_H
#define SCAN_H

#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnDependencyNode.h>

class Scan
{
public:
	Scan();
	~Scan();

	MStatus doScan();

private:

	void findHistory(MString& string, MFnDependencyNode& node);

};

#endif