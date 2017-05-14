#include "mayaUtils.h"

#include <maya/MSelectionList.h>
#include <maya/MUuid.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>

MayaUtils::MayaUtils()
{
}


MayaUtils::~MayaUtils()
{
}


bool MayaUtils::isValidNodeType(MString _type)
{
	return (_type == MString("transform") ||
			_type == MString("mesh") ||
			_type == MString("polyTweak") ||
			_type == MString("polyExtrudeFace") ||
			_type == MString("polySplitRing") ||
			_type == MString("polyCube"));
}

bool MayaUtils::doesItRequireConnections(MString _type)
{
	return (_type == MString("polyTweak") ||
			_type == MString("polySplitRing") ||
			_type == MString("polyExtrudeFace"));
}

MStatus MayaUtils::getNodeObjectFromUUID(MString& uuid, MObject& _node)
{
	// get node
	MSelectionList sList;
	MUuid ID(uuid);
	sList.add(ID);
	return sList.getDependNode(0, _node);
}

bool MayaUtils::doesItExist(std::string _id)
{
	MStatus status;
	MSelectionList selList;
	MUuid id(_id.c_str());
	status = selList.add(id);

	return (status == MStatus::kSuccess);
}

MStatus MayaUtils::getIncomingNodeObject(MFnDependencyNode& node, MFnDependencyNode& incomingNode)
{
	MStatus status;
	MPlug inMeshPlug;
	inMeshPlug = MayaUtils::getInPlug(node, status);

	if (inMeshPlug.isConnected())
	{
		MPlugArray tempPlugArray;
		inMeshPlug.connectedTo(tempPlugArray, true, false);
		// Only one connection should exist on meshNodeShape.inMesh!
		MPlug upstreamNodeSrcPlug = tempPlugArray[0];
		incomingNode.setObject(upstreamNodeSrcPlug.node());
		return MStatus::kSuccess;
	}

	return status;
}

MStatus MayaUtils::getOutgoingNodeObject(MFnDependencyNode& node, MFnDependencyNode& outgoingNode)
{
	MStatus status;
	MPlug outMeshPlug;
	outMeshPlug = MayaUtils::getOutPlug(node, status);

	if (outMeshPlug.isConnected())
	{
		MPlugArray tempPlugArray;
		outMeshPlug.connectedTo(tempPlugArray, false, true);
		MPlug downStreamNodeSrcPlug = tempPlugArray[0];
		outgoingNode.setObject(downStreamNodeSrcPlug.node());
		return MStatus::kSuccess;
	}

	return status;
}

MPlug MayaUtils::getInPlug(MFnDependencyNode& node, MStatus& status)
{
	MPlug in = node.findPlug("inputPolymesh", &status);

	// if it doesnt have that plug try this one
	if (status != MStatus::kSuccess)
	{
		in = node.findPlug("inMesh", &status);
	}

	return in;
}

MPlug MayaUtils::getOutPlug(MFnDependencyNode& node, MStatus &status)
{
	return node.findPlug("output", &status);
}