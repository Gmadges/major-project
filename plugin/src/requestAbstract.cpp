#include "requestAbstract.h"

#include "mayaUtils.h"
#include "tweakHandler.h"

#include <maya/MArgDatabase.h>
#include <maya/MGlobal.h>
#include <maya/MUuid.h>
#include <maya/MFnDependencyNode.h>

RequestAbstract::RequestAbstract()
	:
	pTweakHandler(new TweakHandler())
{
}


RequestAbstract::~RequestAbstract()
{
}

MSyntax RequestAbstract::newSyntax()
{
	MSyntax syn;

	syn.addFlag("-id", "-uuid", MSyntax::kString);

	return syn;
}

MStatus RequestAbstract::getArgs(const MArgList& args, MString& id)
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

MStatus RequestAbstract::setNodeValues(json & _node)
{
	MObject node;
	MString id = _node["id"].get<std::string>().c_str();
	if (MayaUtils::getNodeObjectFromUUID(id, node) != MStatus::kSuccess) return MStatus::kFailure;
	// rename and set correct details
	MFnDependencyNode depNode(node);

	depNode.setName(_node["name"].get<std::string>().c_str());

	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	auto dataAttribs = _node["attribs"];

	return setAttribs(depNode, dataAttribs);
}

MStatus RequestAbstract::setAttribs(MFnDependencyNode& node, json& attribs)
{
	MStatus status;

	for (json::iterator it = attribs.begin(); it != attribs.end(); ++it)
	{
		MPlug plug = node.findPlug(MString(it.key().c_str()), status);

		if (status == MStatus::kSuccess)
		{
			//TODO object
			//TODO null

			if (it.value().is_boolean())
			{
				plug.setBool(it.value());
				continue;
			}

			if (it.value().is_number_float())
			{
				plug.setFloat(it.value());
				continue;
			}

			if (it.value().is_string())
			{
				std::string val = it.value();
				plug.setString(MString(val.c_str()));
				continue;
			}

			if (it.value().is_number_integer())
			{
				plug.setInt(it.value());
				continue;
			}

			if (it.value().is_number_unsigned())
			{
				plug.setInt64(it.value());
				continue;
			}

			if (it.value().is_array())
			{
				if (it.key().compare("tk") == 0)
				{
					std::vector<json> tweakVals = it.value();
					pTweakHandler->setTweakPlugFromArray(plug, tweakVals);
				}

				//TODO all other cases
				continue;
			}

			if (it.value().is_object())
			{
				// TODO
				continue;
			}

			if (it.value().is_null())
			{
				// TODO
				continue;
			}
		}
	}
	return MStatus::kSuccess;
}

MStatus RequestAbstract::createNode(json& _node)
{
	MStatus status;

	std::string nodetype = _node["type"];
	MObject obj = fDGModifier.createNode(MString(nodetype.c_str()), &status);

	if (status != MStatus::kSuccess) return status;

	MFnDependencyNode node;
	node.setObject(obj);
	node.setName(_node["name"].get<std::string>().c_str());
	MUuid id(_node["id"].get<std::string>().c_str());
	node.setUuid(id);

	return fDGModifier.doIt();
}

MStatus RequestAbstract::setConnections(json& _mesh, json& _node)
{
	MStatus status;

	// TODO rewrite to connect based on in and out connections.

	// if its not a mesh we'll have to wire it in
	std::string type = _node["type"];

	if (type.compare("polySplitRing") == 0 ||
		type.compare("polyTweak") == 0)
	{
		// get mesh and set it to be the one we're effecting
		MString meshID(_mesh["id"].get<std::string>().c_str());
		MObject meshNode;
		status = MayaUtils::getNodeObjectFromUUID(meshID, meshNode);
		if (status != MStatus::kSuccess) return status;

		MDagPath dagPath;
		MDagPath::getAPathTo(meshNode, dagPath);
		dagPath.extendToShape();
		setMeshNode(dagPath);

		// get node and do the connections
		MString nodeID(_node["id"].get<std::string>().c_str());
		MObject node;
		status = MayaUtils::getNodeObjectFromUUID(nodeID, node);
		if (status != MStatus::kSuccess) return status;

		//// and add it to the DAG
		status = doModifyPoly(node);
		if (status != MStatus::kSuccess) return status;

		// this is if we require extra connections
		if (type.compare("polySplitRing") == 0)
		{
			MString connectCmd;
			connectCmd += "connectAttr ";
			std::string meshName = _mesh["name"];
			connectCmd += meshName.c_str();
			connectCmd += ".worldMatrix[0] ";
			std::string nodeName = _node["name"];
			connectCmd += nodeName.c_str();
			connectCmd += ".manipMatrix;";
			MGlobal::executeCommand(connectCmd);
		}
	}

	return MStatus::kSuccess;
}


