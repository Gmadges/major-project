#ifndef REQUESTUPDATE_H
#define REQUESTUPDATE_H

#include "requestAbstract.h"

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <memory>

class Messaging;

class RequestUpdate : public RequestAbstract
{
public:
	RequestUpdate();
	~RequestUpdate();

	static void* creator();
	MStatus	doIt(const MArgList&);

private:
	std::unique_ptr<Messaging> pMessenger;
};

#endif