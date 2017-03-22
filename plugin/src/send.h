#ifndef SEND_H
#define SEND_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

#include <memory>
#include "json.h"

class Messaging;
class TweakHandler;

class Send : public MPxCommand
{
public:
	Send();
	~Send();

	static void* creator();
	static MSyntax newSyntax();
	virtual MStatus	doIt(const MArgList&);

private:
	MStatus getArgs(const MArgList& args, MString& address, int& port);

	std::unique_ptr<Messaging> pMessaging;
	std::unique_ptr<TweakHandler> pTweaksHandler;
};

#endif