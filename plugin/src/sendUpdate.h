#ifndef SENDUPDATE_H
#define SENDUPDATE_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

#include <memory>
#include "json.h"

#include "sendAbstract.h"

class Messaging;
class TweakHandler;

class SendUpdate : public SendAbstract
{
public:
	SendUpdate();
	~SendUpdate();

	// have to have a creator
	static void* creator();
	virtual MStatus	doIt(const MArgList&) override;
};

#endif