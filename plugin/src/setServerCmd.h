#ifndef SETSERVERCMD_H
#define SETSERVERCMD_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

class SetServerCmd : public MPxCommand
{
public:
	SetServerCmd();
	~SetServerCmd();

	static void* creator();
	static MSyntax newSyntax();
	virtual MStatus doIt(const MArgList&);

private:

	MStatus getArgs(const MArgList& args, bool& query, MString& address, int& port, MString& uid);
};

#endif