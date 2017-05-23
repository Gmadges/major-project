#include "setServerCmd.h"

#include <maya/MArgDatabase.h>

#include "hackPrint.h"
#include "dataStore.h"

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

	syn.addFlag("-q", "-query");
	syn.addFlag("-a", "-address", MSyntax::kString);
	syn.addFlag("-p", "-port", MSyntax::kUnsigned);
	syn.addFlag("-uid", "-userid", MSyntax::kString);

	return syn;
}

MStatus SetServerCmd::doIt(const MArgList& args)
{
	MStatus status;

	// reset socket
	MString addr, uid;
	int port;
	bool query = false;
	status = getArgs(args, query, addr, port, uid);

	if (query)
	{
		MStringArray result;
		if (DataStore::getInstance().isServerSet())
		{
			MString portString;
			portString += DataStore::getInstance().getPort();
			result.append(portString);

			result.append(DataStore::getInstance().getAddress().c_str());
			result.append(DataStore::getInstance().getUserID().c_str());
		}
		setResult(result);
		return MStatus::kSuccess;
	}

	if (status != MStatus::kSuccess)
	{
		HackPrint::print("all flags are required");
		return status;
	}

	DataStore::getInstance().setPort(port);
	DataStore::getInstance().setIPAddress(addr.asChar());
	DataStore::getInstance().setUserID(uid.asChar());
	return MS::kSuccess;
}

MStatus SetServerCmd::getArgs(const MArgList& args, bool& query, MString& address, int& port, MString& uid)
{
	MStatus status = MStatus::kSuccess;
	MArgDatabase parser(syntax(), args, &status);

	if (status != MS::kSuccess) return status;

	if (parser.isFlagSet("-q"))
	{
		query = true;
	}

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

	if (parser.isFlagSet("-uid"))
	{
		parser.getFlagArgument("-uid", 0, uid);
	}
	else
	{
		status = MStatus::kFailure;
	}

	return status;
}
