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
#include "dataStore.h"
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

	syn.addFlag("-fm", "-fullMesh", MSyntax::kBoolean);

	return syn;
}

MStatus RequestUpdate::getArgs(const MArgList& args, bool& forceFullMesh)
{
	MStatus status = MStatus::kSuccess;
	MArgDatabase parser(syntax(), args, &status);

	if (status != MS::kSuccess) return status;

	if (parser.isFlagSet("-fm"))
	{
		parser.getFlagArgument("-fm", 0, forceFullMesh);
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
	if (!DataStore::getInstance().isServerSet())
	{
		HackPrint::print("Set Server using \"SetServer\" command");
		return status;
	}

	pMessenger->resetSocket(DataStore::getInstance().getAddress(), DataStore::getInstance().getPort());

	bool bFullMesh = false;
	getArgs(args, bFullMesh);

	json data;
	// if false then we couldnt connect to server
	if (!pMessenger->requestMesh(data,
									ReqType::REQUEST_MESH_UPDATE,
									DataStore::getInstance().getCurrentRegisteredMesh(),
									DataStore::getInstance().getUserID(),
									bFullMesh))
	{
		return MStatus::kFailure;
	}

	// is there actually anything?
	if (data.empty() || data["edits"].empty())
	{
		return status;
	}

	// we should stop listening for changes because otherwise we'll keep sending back the changes we just made.
	CallbackHandler::getInstance().setIgnoreChanges(true);

	if (bFullMesh)
	{
		status = applyNodes(data["edits"]);
	}
	else
	{
		status = applyEdits(data["edits"]);
	}

	// we should stop listening for changes because otherwise we'll keep sending back the changes we just made.
	CallbackHandler::getInstance().setIgnoreChanges(false);

	return status;
}

MStatus RequestUpdate::applyNodes(std::vector<json> nodeList)
{
	MStatus status;

	for (auto& itr : nodeList)
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

				// set the values
				status = setNodeValues(itr);
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

		// TODO
		// think about what happens to nodes that we dont get a delete message for if we're in fullmesh mode
    }

	return status;
}

MStatus RequestUpdate::applyEdits(std::vector<json> editList)
{
	MStatus status;

	for (auto& edit : editList)
	{
		status = applyNodes(edit["nodes"]);
	}

	return status;
}

MStatus RequestUpdate::deleteNode(MObject& node)
{
	MStatus status = fDGModifier.deleteNode(node);
	if (status != MStatus::kSuccess) return status;
	status = fDGModifier.doIt();
	return status;
}
