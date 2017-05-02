#include "requestUpdate.h"
#include "hackPrint.h"
#include "messaging.h"

#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"
#include "maya/MDagPathArray.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MPlug.h"
#include "maya/MArgDatabase.h"
#include "maya/MPlugArray.h"
#include "maya/MUuid.h"

#include "testTypes.h"
#include "tweakHandler.h"
#include "serverAddress.h"
#include "callbackHandler.h"
#include "mayaUtils.h"

RequestUpdate::RequestUpdate()
	:
	pMessenger(new Messaging("localhost", 8080))
{
}

RequestUpdate::~RequestUpdate()
{
}

void* RequestUpdate::creator()
{
	return new RequestUpdate;
}

MSyntax RequestUpdate::newSyntax()
{
	MSyntax syn;

	syn.addFlag("-id", "-uuid", MSyntax::kString);

	return syn;
}

MStatus RequestUpdate::getArgs(const MArgList& args, MString& id)
{
	MStatus status = MStatus::kSuccess;
	MArgDatabase parser(syntax(), args, &status);

	if (status != MS::kSuccess) return status;

	if (parser.isFlagSet("-id"))
	{
		parser.getFlagArgument("-id", 0, id);
	}
	else
	{
		status = MStatus::kFailure;
	}

	return status;
}

MStatus	RequestUpdate::doIt(const MArgList& args)
{
	MStatus status = MStatus::kSuccess;

	// reset socket
	if (!ServerAddress::getInstance().isServerSet())
	{
		HackPrint::print("Set Server using \"SetServer\" command");
		return status;
	}

	pMessenger->resetSocket(ServerAddress::getInstance().getAddress(), ServerAddress::getInstance().getPort());

	// ask the server for any update
	json data;
	MString id;
	if (getArgs(args, id) != MStatus::kSuccess)
	{
		HackPrint::print("no id specified!");
		return MStatus::kFailure;
	}

	// if false then we couldnt connect to server
	if (!pMessenger->requestMesh(data, ReqType::REQUEST_MESH_UPDATE, std::string(id.asChar()), ServerAddress::getInstance().getUserID())) return MStatus::kFailure;

	// is there actually anything?
	if (data.empty())
	{
		HackPrint::print("Nothing to update");
		return status;
	}
	else if (data["edits"].empty())
	{
		HackPrint::print("No Edits");
		return status;
	}

	HackPrint::print(data.dump(4));

	return status;
}
