#ifndef UPDATE_H
#define UPDATE_H

#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnDependencyNode.h>

class Update
{
public:
	Update();
	~Update();

	MStatus doScan();

private:
	void findHistory(MString& string, MFnDependencyNode& node);
};

#endif