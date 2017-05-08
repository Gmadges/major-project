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
	MStatus status = MStatus::kSuccess;

	// if its not a mesh we'll have to wire it in
	std::string type = _node["type"];

	if (type.compare("polySplitRing") == 0 ||
		type.compare("polyTweak") == 0)
	{
		// redo of our connecting code.

		MFnDependencyNode inNode, outNode, newNode;

		// grab the in and out nodes from this node
		if (_node["in"] != nullptr)
		{
			MString inNodeID(_mesh["in"].get<std::string>().c_str());
			MObject tmp;
			status = MayaUtils::getNodeObjectFromUUID(inNodeID, tmp);
			if (status != MStatus::kSuccess) return status;
			inNode.setObject(tmp);
		}

		if (_node["out"] != nullptr)
		{
			MString outNodeID(_mesh["out"].get<std::string>().c_str());
			MObject tmp;
			status = MayaUtils::getNodeObjectFromUUID(outNodeID, tmp);
			if (status != MStatus::kSuccess) return status;
			outNode.setObject(tmp);
		}

		MString newNodeID(_mesh["id"].get<std::string>().c_str());
		MObject tmp;
		status = MayaUtils::getNodeObjectFromUUID(newNodeID, tmp);
		if (status != MStatus::kSuccess) return status;
		newNode.setObject(tmp);
		
		// check what connections we have and siconnect if we need to
		if (!inNode.object().isNull())
		{
			MFnDependencyNode tmpDepNode;
			if (MayaUtils::getOutgoingNodeObject(inNode, tmpDepNode) == MStatus::kSuccess)
			{
				status = disconnectNodes(inNode, tmpDepNode);
			}
		}

		// most of the time this shouldnt have to happen because we just disconnected the node its most likely to be connected to
		if (!outNode.object().isNull())
		{
			MFnDependencyNode tmpDepNode;
			if (MayaUtils::getIncomingNodeObject(outNode, tmpDepNode) == MStatus::kSuccess)
			{
				status = disconnectNodes(tmpDepNode, outNode);
			}
		}

		// attach everything

		MPlug nodeOutPlug = MayaUtils::getOutPlug(newNode, status);
		MPlug nodeInPlug = MayaUtils::getInPlug(newNode, status);

		MPlug inNodePlug = MayaUtils::getOutPlug(inNode, status);
		MPlug outNodePlug = MayaUtils::getInPlug(outNode, status);

		status = fDGModifier.connect(inNodePlug, nodeInPlug);
		status = fDGModifier.connect(nodeOutPlug, outNodePlug);
		
		status = fDGModifier.doIt();

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

	return status;
}

MStatus RequestAbstract::disconnectNodes(MFnDependencyNode& inNode, MFnDependencyNode& outnode)
{
	MStatus status;
	MPlug out = MayaUtils::getOutPlug(outnode, status);
	MPlug in = MayaUtils::getInPlug(inNode, status);

	status = fDGModifier.disconnect(out, in);

	if (status != MStatus::kSuccess) return status;

	return fDGModifier.doIt();
}
