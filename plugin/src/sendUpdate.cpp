#include "sendUpdate.h"

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnAttribute.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MUuid.h>

#include <string>

#include "messaging.h"
#include "tweakHandler.h"
#include "hackPrint.h"
#include "testTypes.h"

#include "callbackHandler.h"

SendUpdate::SendUpdate()
	:
	SendAbstract()
{
}

SendUpdate::~SendUpdate()
{
}

void* SendUpdate::creator()
{
	return new SendUpdate;
}

MStatus	SendUpdate::doIt(const MArgList& args)
{
	MStatus status;

	// check for args at some point

	// get list of things we need to send
	auto list = CallbackHandler::getInstance().getSendList();
	
	for (auto& itr : list)
	{
		std::string val;

		val += itr.first;
		val += " : ";
		val += std::to_string(itr.second);

		HackPrint::print(val);
	}

	// clear the list
	CallbackHandler::getInstance().resetSendList();

	return MS::kSuccess;
}