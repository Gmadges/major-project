#include "update.h"
#include "hackPrint.h"
#include "messaging.h"

#include "genericMessage.h"

#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MPlug.h"

Update::Update()
	:
	pMessenger(new Messaging("8080"))
{
}

Update::~Update()
{
}

void* Update::creator()
{
	return new Update;
}

MStatus	Update::doIt(const MArgList& args)
{
	MStatus status = MStatus::kSuccess;

	// ask the server for any update
	GenericMessage data;
	
	// if false then we couldnt connect to server
	if (!pMessenger->requestData(data)) return MStatus::kFailure;
	
	// is there actually anything?
	if (data.getNodeType().empty())
	{
		HackPrint::print("Nothing to update");
		return status;
	}

	// create a node of same type?
	MString cmd;
	cmd += "createNode \"";
	cmd += data.getNodeType().c_str();
	cmd += "\"";

	MString resultNodeName;
	status = MGlobal::executeCommand(cmd, resultNodeName);
	if (!status) return status;

	// lets get the node
	MSelectionList selectList;
	selectList.add(resultNodeName);
	MGlobal::getActiveSelectionList(selectList);

	MDagPath d;
	selectList.getDagPath(0, d);
	MFnDependencyNode node(d.node());

	setNodeValues(node, data);

	// and add it to the DAG

	return status;
}

void Update::setNodeValues(MFnDependencyNode & node, GenericMessage & data)
{
	// rename and set correct details
	node.setName(MString(data.getName().c_str()));

	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	auto dataAttribs = data.getAttribs();
	MStatus status;

	for ( auto atr : dataAttribs )
	{
		MPlug plug = node.findPlug(atr.first.c_str(), status);

		if (status == MStatus::kSuccess)
		{
			plug.setValue(atr.second.c_str());
		}
	}
}
