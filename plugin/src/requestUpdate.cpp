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

	json data;
	
	// if false then we couldnt connect to server
	if (!pMessenger->requestMesh(data, ReqType::REQUEST_MESH_UPDATE, CallbackHandler::getInstance().getCurrentRegisteredMesh(), ServerAddress::getInstance().getUserID())) return MStatus::kFailure;

	// is there actually anything?
	if (data.empty() || data["edits"].empty())
	{
		return status;
	}

	// we should stop listening for changes because otherwise we'll keep sending back the changes we just made.
	CallbackHandler::getInstance().ignoreChanges(true);

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
					status = setConnections(itr);
				}
			}
			else if (itr["edit"] == EditType::EDIT)
			{
				status = setNodeValues(itr);
			}
			else if (itr["edit"] == EditType::DEL)
			{
				MObject node;
				MString id = itr["id"].get<std::string>().c_str();
				status = MayaUtils::getNodeObjectFromUUID(id, node);
				deleteNode(node);
			}

			// TODO check connection order possibly?
		}
	}

	// we should stop listening for changes because otherwise we'll keep sending back the changes we just made.
	CallbackHandler::getInstance().ignoreChanges(false);

	return status;
}

MStatus RequestUpdate::deleteNode(MObject& node)
{
	MStatus status = fDGModifier.deleteNode(node);
	if (status != MStatus::kSuccess) return status;
	status = fDGModifier.doIt();
	return status;
}
