#include "sendAbstract.h"

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MString.h>
#include <maya/MFnMesh.h>
#include <maya/MArgList.h>
#include <maya/MPoint.h>
#include <maya/MIOStream.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnAttribute.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MUuid.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>

#include "messaging.h"
#include "tweakHandler.h"
#include "hackPrint.h"
#include "testTypes.h"

#include "callbackHandler.h"
#include "mayaUtils.h"

SendAbstract::SendAbstract()
	:
	pMessaging(new Messaging("localhost", 8080)),
	pTweaksHandler(new TweakHandler())
{
}


SendAbstract::~SendAbstract()
{
}

void SendAbstract::traverseAllValidNodesForMesh(MDagPath& dagPath, std::function<void(MFnDependencyNode&)>& func)
{
	// first do the transforms node
	MFnDependencyNode transformNode(dagPath.transform());
	func(transformNode);

	// now the rest of the meshes history
	dagPath.extendToShape();
	MFnDependencyNode meshShapeNode(dagPath.node());

	traverseAllValidNodes(meshShapeNode, func);
}

void SendAbstract::traverseAllValidNodes(MFnDependencyNode & node, std::function<void(MFnDependencyNode&)>& func)
{
	if (MayaUtils::isValidNodeType(node.typeName()))
	{
		func(node);
	}

	// now lets see if it has a parent

	// If the inpuPolymesh is connected, we have history
	MStatus status;
	MPlug inMeshPlug;
	inMeshPlug = MayaUtils::getInPlug(node, status);

	if (inMeshPlug.isConnected())
	{
		MPlugArray tempPlugArray;
		inMeshPlug.connectedTo(tempPlugArray, true, false);
		// Only one connection should exist on meshNodeShape.inMesh!
		MPlug upstreamNodeSrcPlug = tempPlugArray[0];
		MFnDependencyNode upstreamNode(upstreamNodeSrcPlug.node());

		traverseAllValidNodes(upstreamNode, func);
	}
}

MStatus SendAbstract::getGenericNode(MFnDependencyNode & _inNode, json& _outNode)
{
	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	unsigned int numAttrib = _inNode.attributeCount();
	MStatus status;

	json nodeAttribs;

	for (unsigned int i = 0; i < numAttrib; i++)
	{
		MFnAttribute attrib(_inNode.attribute(i));

		MPlug plug = _inNode.findPlug(attrib.shortName().asChar(), &status);

		if (status != MStatus::kSuccess) continue;


		getAttribFromPlug(plug, nodeAttribs);
	}

	if (nodeAttribs.empty()) return MStatus::kFailure;

	// uuid
	// get the uuid of the node for sending broseph
	MUuid idObj = _inNode.uuid();

	_outNode["id"] = std::string(idObj.asString().asChar());
	_outNode["name"] = std::string(_inNode.name().asChar());
	_outNode["type"] = std::string(_inNode.typeName().asChar());
	_outNode["time"] = std::time(nullptr);
	_outNode["attribs"] = nodeAttribs;

	// use plugs to get node ordering
	_outNode["out"] = nullptr;
	_outNode["in"] = nullptr;

	MString inID;
	if (getIncomingID(_inNode, inID) == MStatus::kSuccess)
	{
		_outNode["in"] = std::string(inID.asChar());
	}

	MString outID;
	if (getOutgoingID(_inNode, outID) == MStatus::kSuccess)
	{
		_outNode["out"] = std::string(outID.asChar());
	}

	return MStatus::kSuccess;
}

MStatus SendAbstract::getIncomingID(MFnDependencyNode & _inNode, MString& _id)
{
	MFnDependencyNode upstreamNode;
	if (MayaUtils::getIncomingNodeObject(_inNode, upstreamNode) == MStatus::kSuccess)
	{
		_id = upstreamNode.uuid().asString();
		return MStatus::kSuccess;
	}

	return MStatus::kFailure;
}

MStatus SendAbstract::getOutgoingID(MFnDependencyNode & _inNode, MString& _id)
{
	MFnDependencyNode downStreamNode;
	if (MayaUtils::getOutgoingNodeObject(_inNode, downStreamNode) == MStatus::kSuccess)
	{
		_id = downStreamNode.uuid().asString();
		return MStatus::kSuccess;
	}

	return MStatus::kFailure;
}

MStatus SendAbstract::getAttribFromPlug(MPlug& _plug, json& _attribs)
{
	std::string attribName = _plug.partialName().asChar();

	if (_plug.isNull())
	{
		_attribs[attribName] = nullptr;
		return MStatus::kSuccess;
	}

	//hack stops crashing on plugs which have an index of -1
	if (attribName.find("-1") != std::string::npos)
	{
		return MStatus::kFailure;
	}

	if (_plug.isArray())
	{
		std::vector<json> plugArray;
		for (unsigned int i = 0; i < _plug.numElements(); i++)
		{
			// get the MPlug for the i'th array element
			MPlug elemPlug = _plug.elementByPhysicalIndex(i);

			json vals;
			if (getAttribFromPlug(elemPlug, vals) == MStatus::kSuccess)
			{
				plugArray.push_back(vals);
			}
		}
		_attribs[attribName] = plugArray;
		return MStatus::kSuccess;
	}

	if (_plug.isCompound())
	{
		// if the plug is a compound then it has a number of children plugs we need to grab
		unsigned int numChild = _plug.numChildren();
		std::vector<json> childrenPlugs(numChild);
		for (unsigned int i = 0; i < numChild; ++i)
		{
			MPlug childPlug = _plug.child(i);

			// get values
			// and store too
			json vals;
			if (getAttribFromPlug(childPlug, vals) == MStatus::kSuccess)
			{
				childrenPlugs.push_back(vals);
			}
		}
		_attribs[attribName] = childrenPlugs;
		return MStatus::kSuccess;
	}

	//MString value;
	float fValue;
	if (_plug.getValue(fValue) == MStatus::kSuccess)
	{
		_attribs[attribName] = fValue;
		return MStatus::kSuccess;
	}

	double dValue;
	if (_plug.getValue(dValue) == MStatus::kSuccess)
	{
		_attribs[attribName] = dValue;
		return MStatus::kSuccess;
	}

	MString sValue;
	if (_plug.getValue(sValue) == MStatus::kSuccess)
	{
		_attribs[attribName] = sValue.asChar();
		return MStatus::kSuccess;
	}

	int iValue;
	if (_plug.getValue(iValue) == MStatus::kSuccess)
	{
		_attribs[attribName] = iValue;
		return MStatus::kSuccess;
	}

	bool bValue;
	if (_plug.getValue(bValue) == MStatus::kSuccess)
	{
		_attribs[attribName] = bValue;
		return MStatus::kSuccess;
	}

	//// TODO more maya data types
	//MAngle MAngleValue();
	//if (_plug.getValue(bValue) == MStatus::kSuccess)
	//{
	//	_attribs.insert(attribType(attribName, msgpack::object(bValue)));
	//	return MStatus::kSuccess;
	//}

	//MDistance DistenceValue();
	//if (_plug.getValue(bValue) == MStatus::kSuccess)
	//{
	//	_attribs.insert(attribType(attribName, msgpack::object(bValue)));
	//	return MStatus::kSuccess;
	//}

	//MTime TimeValue();
	//if (_plug.getValue(bValue) == MStatus::kSuccess)
	//{
	//	_attribs.insert(attribType(attribName, msgpack::object(bValue)));
	//	return MStatus::kSuccess;
	//}

	//// TODO look at handling data and objects possibly.

	return MStatus::kFailure;
}