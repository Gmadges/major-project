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

	// cycle through edits

	// figure out if we need to create or delete something

	// create in the correct place or move to right place

	// store values in the node.

	// done i guess

	// get edits

	auto editList = data["edits"];

	for (auto& edit : editList)
	{
		for (auto& itr : edit["nodes"])
		{

			if (itr["edit"] == EditType::ADD)
			{
				std::string stringID = itr["id"];
				bool nodeExists = MayaUtils::doesItExist(stringID);

				if (!nodeExists)
				{
					status = createNode(itr);
					MObject node;
					MString id = itr["id"].get<std::string>().c_str();
					status = MayaUtils::getNodeObjectFromUUID(id, node);
					CallbackHandler::getInstance().registerCallbacksToNode(node);

					// connect
					status = setConnections(edit, itr);
				}
			}
			else if (itr["edit"] == EditType::EDIT)
			{
				status = setNodeValues(itr);
			}
			else if (itr["edit"] == EditType::DEL)
			{
				// TODO delete node
			}

			// TODO check connections
		}
	}

	return status;
}
