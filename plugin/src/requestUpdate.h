#ifndef REQUESTUPDATE_H
#define REQUESTUPDATE_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <memory>

class Messaging;

class RequestUpdate : public MPxCommand
{
public:
	RequestUpdate();
	~RequestUpdate();

	static void* creator();
	static MSyntax newSyntax();
	MStatus	doIt(const MArgList&);

private:
	MStatus getArgs(const MArgList& args, MString& id);

private:
	std::unique_ptr<Messaging> pMessenger;
};

#endif