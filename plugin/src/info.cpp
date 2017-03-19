#include "info.h"

#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

#include "messaging.h"
#include "hackPrint.h"
#include "testTypes.h"

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

MSyntax Info::newSyntax()
{

	MSyntax syn;

	syn.addFlag("-a", "-address", MSyntax::kString);
	syn.addFlag("-p", "-port", MSyntax::kUnsigned);

	return syn;
}

MStatus Info::doIt(const MArgList& args)
{
	MStatus status;
	MStringArray result;

	// reset socket
	MString addr;
	int port;
	status = getArgs(args, addr, port);
	if (status != MStatus::kSuccess)
	{
		HackPrint::print("no input values specified");
		setResult(result);
		return status;
	}

	pMessaging->resetSocket(std::string(addr.asChar()), port);

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

MStatus Info::getArgs(const MArgList& args, MString& address, int& port)
{
	MStatus status = MStatus::kSuccess;
	MArgDatabase parser(syntax(), args, &status);

	if (status != MS::kSuccess) return status;

	// get the command line arguments that were specified
	if (parser.isFlagSet("-p"))
	{
		parser.getFlagArgument("-p", 0, port);
	}
	else
	{
		status = MStatus::kFailure;
	}

	if (parser.isFlagSet("-a"))
	{
		parser.getFlagArgument("-a", 0, address);
	}
	else
	{
		status = MStatus::kFailure;
	}

	return status;
}