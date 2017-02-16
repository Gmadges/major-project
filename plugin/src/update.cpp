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

	//TEST
	cmd += " -n \"TmpNode\"";

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

	//setNodeValues(depNode, data);

	// TEST
	// hard code
	MSelectionList selList;
	selList.add("pCube1|pCubeShape1");
	MDagPath dagpath;
	selList.getDagPath(0, dagpath);
	dagpath.extendToShape();
	setMeshNode(dagpath);

	HackPrint::print(dagpath.fullPathName());

	//// and add it to the DAG
	doModifyPoly(newNode);

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
		if (atr.first.compare("out") == 0 || atr.first.compare("ip") == 0) continue;

		MPlug plug = node.findPlug(atr.first.c_str(), status);

		if (status == MStatus::kSuccess)
		{
			switch (atr.second.type)
			{
				case msgpack::type::BOOLEAN:
				{
					//plug.setBool(atr.second.via.boolean);
					break;
				}
				case msgpack::type::FLOAT:
				{
					//plug.setFloat(atr.second.via.f64);
					break;
				}
				case msgpack::type::STR :
				{
					std::string val;
					atr.second.convert(val);
					//plug.setString(MString(val.c_str()));
					break;
				}
				case msgpack::type::NEGATIVE_INTEGER:
				{
					//plug.setInt(atr.second.via.i64);
					break;
				}
				case msgpack::type::POSITIVE_INTEGER:
				{
					//plug.setInt(atr.second.via.i64);
					break;
				}
			}
		}
	}
}
