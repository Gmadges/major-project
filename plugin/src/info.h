#ifndef INFO_H
#define INFO_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

#include <memory>
#include "json.h"

class Messaging;

class Info : public MPxCommand
{
public:
	Info();
	~Info();

	static void* creator();
	static MSyntax newSyntax();
	virtual MStatus doIt(const MArgList&);

private:

	MStatus getArgs(const MArgList& args, MString& address, int& port);

private:
	std::unique_ptr<Messaging> pMessaging;
};

#endif