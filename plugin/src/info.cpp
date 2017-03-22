#include "info.h"

#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

#include "messaging.h"
#include "hackPrint.h"
#include "testTypes.h"
#include "serverAddress.h"

Info::Info()
	:
	pMessaging(new Messaging("localhost", 8080))
{
}

Info::~Info()
{
}

void* Info::creator()
{
	return new Info;
}

MStatus Info::doIt(const MArgList& args)
{
	MStatus status;
	MStringArray result;

	if (!ServerAddress::getInstance().isServerSet())
	{
		HackPrint::print("Set Server using \"SetServer\" command");
		return status;
	}

	pMessaging->resetSocket(ServerAddress::getInstance().getAddress(), ServerAddress::getInstance().getPort());

	// ask for the info
	json data;
	// if false then we couldnt connect to server
	if (!pMessaging->requestData(data, ReqType::INFO_REQUEST))
	{
		setResult(result);
		return MStatus::kFailure;
	}

	// turn it into a string array to return
	for (auto& val : json::iterator_wrapper(data))
	{
		result.append(MString(val.value().get<std::string>().c_str()));
		result.append(MString(val.key().c_str()));
	}

	setResult(result);

	return MS::kSuccess;
}