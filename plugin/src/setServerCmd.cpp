#include "setServerCmd.h"

#include <maya/MArgDatabase.h>

#include "hackPrint.h"
#include "serverAddress.h"

SetServerCmd::SetServerCmd()
{
}

SetServerCmd::~SetServerCmd()
{
}


void* SetServerCmd::creator()
{
	return new SetServerCmd;
}

MSyntax SetServerCmd::newSyntax()
{

	MSyntax syn;

	syn.addFlag("-a", "-address", MSyntax::kString);
	syn.addFlag("-p", "-port", MSyntax::kUnsigned);

	return syn;
}

MStatus SetServerCmd::doIt(const MArgList& args)
{
	MStatus status;

	// reset socket
	MString addr;
	int port;
	status = getArgs(args, addr, port);
	if (status != MStatus::kSuccess)
	{
		HackPrint::print("both address and port is needed!");
		return status;
	}

	ServerAddress::getInstance().setPort(port);
	ServerAddress::getInstance().setAddress(addr.asChar());

	return MS::kSuccess;
}

MStatus SetServerCmd::getArgs(const MArgList& args, MString& address, int& port)
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
