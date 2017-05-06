#ifndef CLEARCURRENTMESH_H
#define CLEARCURRENTMESH_H

#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MString.h>

class ClearCurrentMesh : public MPxCommand
{
public:
	ClearCurrentMesh();
	~ClearCurrentMesh();

	static void* creator();
	virtual MStatus doIt(const MArgList&);
};

#endif