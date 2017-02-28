#include "update.h"
#include "hackPrint.h"
#include "messaging.h"

#include "genericMessage.h"

#include "maya/MSelectionList.h"
#include "maya/MDagPath.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MPlug.h"
#include "maya/MArgDatabase.h"

Update::Update()
	:
	pMessenger(new Messaging("localhost",8080))
{
}

Update::~Update()
{
}

void* Update::creator()
{
	return new Update;
}

MSyntax Update::newSyntax()
{
	MSyntax syn;

	syn.addFlag("-a", "-address", MSyntax::kString);
	syn.addFlag("-p", "-port", MSyntax::kUnsigned);

	return syn;
}

MStatus	Update::doIt(const MArgList& args)
{
	MStatus status = MStatus::kSuccess;

	// reset socket
	MString addr;
	int port;
	status = getArgs(args, addr, port);
	if (status != MStatus::kSuccess)
	{
		HackPrint::print("no input values specified");
		return status;
	}

	pMessenger->resetSocket(std::string(addr.asChar()), port);

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

	// check if node exists
	MString objExistsCmd;
	objExistsCmd += "objExists \"";
	objExistsCmd += data.getNodeName().c_str();
	objExistsCmd += "\"";

	HackPrint::print(objExistsCmd);

	int exists = 0;
	status = MGlobal::executeCommand(objExistsCmd, exists);
	if (status != MStatus::kSuccess) return status;

	if (!exists)
	{
		HackPrint::print("Node doesnt exist");

		// create a node of same type?
		MString cmd;
		cmd += "createNode \"";
		cmd += data.getNodeType().c_str();
		cmd += "\"";

		// node name
		cmd += " -n \"";
		cmd += data.getNodeName().c_str();
		cmd +=	"\"";

		status = MGlobal::executeCommand(cmd);
		if (status != MStatus::kSuccess) return status;

		MSelectionList sList;
		sList.add(data.getNodeName().c_str());

		MObject newNode;
		status = sList.getDependNode(0, newNode);
		HackPrint::print(newNode.apiTypeStr());
		if (!status) return status;

		setNodeValues(newNode, data);

		// if its not a mesh we'll have to wire it in
		if (data.getNodeType().compare("polyCube") != 0)
		{
			MSelectionList selList;
			selList.add(MString(data.getMeshName().c_str()));
			MDagPath dagpath;
			selList.getDagPath(0, dagpath);
			dagpath.extendToShape();
			setMeshNode(dagpath);

			//// and add it to the DAG
			doModifyPoly(newNode);

			// this check for now
			if (data.getNodeType().compare("polySplitRing") == 0)
			{
				MString connectCmd;
				connectCmd += "connectAttr ";
				connectCmd += data.getMeshName().c_str();
				connectCmd += ".worldMatrix[0] ";
				connectCmd += data.getNodeName().c_str();
				connectCmd += ".manipMatrix;";
				MGlobal::executeCommand(connectCmd);
			}
		}

		return status;
	}

	HackPrint::print("node exists");

	// no need to worry about re-wireing anything
	// just change the values
	MSelectionList sList;
	sList.add(data.getNodeName().c_str());
	MObject node;
	status = sList.getDependNode(0, node);
	if (status != MStatus::kSuccess) return status;
	
	setNodeValues(node, data);

	return status;
}

MStatus Update::getArgs(const MArgList& args, MString& address, int& port)
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

void Update::setNodeValues(MObject & node, GenericMessage & data)
{
	// rename and set correct details
	MFnDependencyNode depNode(node);

	depNode.setName(MString(data.getNodeName().c_str()));

	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	auto dataAttribs = data.getAttribs();
	MStatus status;

	for ( auto atr : dataAttribs )
	{
		//if (atr.first.compare("out") == 0 || atr.first.compare("ip") == 0) continue;
		//if (atr.first.compare("axx") != 0 ) continue;

		MPlug plug = depNode.findPlug(atr.first.c_str(), status);

		if (status == MStatus::kSuccess)
		{
			//HackPrint::print(plug.name());

			switch (atr.second.type)
			{
				case msgpack::type::BOOLEAN:
				{
					//HackPrint::print("bool");
					plug.setBool(atr.second.via.boolean);
					break;
				}
				case msgpack::type::FLOAT:
				{
					//HackPrint::print("float");
					//HackPrint::print(std::to_string(atr.second.via.f64));
					plug.setFloat(atr.second.via.f64);
					break;
				}
				case msgpack::type::STR :
				{
					std::string val;
					atr.second.convert(val);
					//HackPrint::print(val);
					plug.setString(MString(val.c_str()));
					break;
				}
				case msgpack::type::NEGATIVE_INTEGER:
				{
					//HackPrint::print("Neg Int");
					int val;
					atr.second.convert(val);
					//HackPrint::print(std::to_string(val));
					plug.setInt(val);
					break;
				}
				case msgpack::type::POSITIVE_INTEGER:
				{
					//HackPrint::print("Pos int");
					//HackPrint::print(std::to_string(atr.second.via.i64));
					plug.setInt64(atr.second.via.i64);
					break;
				}
			}
		}
	}
}
