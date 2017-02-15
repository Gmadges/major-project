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
	HackPrint::print(resultNodeName);
	
	MSelectionList sList;
	sList.add(resultNodeName);

	MObject newNode;
	status = sList.getDependNode(0, newNode);
	HackPrint::print(newNode.apiTypeStr());
	if (!status) return status;

	MFnDependencyNode depNode(newNode);

	//setNodeValues(depNode, data);

	//// TEST
	//// hard code
	//MSelectionList selList;
	//MString toMatch("pCube*|pCubeShape*");
	//MGlobal::getSelectionListByName(toMatch, selList);
	//MDagPath dagpath;
	//selList.getDagPath(0, dagpath);
	//setMeshNode(dagpath);

	//// and add it to the DAG
	//doModifyPoly(node.object());

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
