#include "mayaUtils.h"

#include <maya/MSelectionList.h>
#include <maya/MUuid.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include "hackPrint.h"

MayaUtils::MayaUtils()
{
}


MayaUtils::~MayaUtils()
{
}


bool MayaUtils::isValidNodeType(MString& _type)
{
	return (_type == MString("transform") ||
			_type == MString("mesh") ||
			_type == MString("polyTweak") ||
			_type == MString("polyExtrudeFace") ||
			_type == MString("polySplitRing") ||
			_type == MString("polySplit") ||
			_type == MString("polyCube") ||
			_type == MString("polyPipe") || 
			_type == MString("polySphere") || 
			_type == MString("polyPyramid") || 
			_type == MString("polyCylinder") ||
			_type == MString("polyTorus") || 
			_type == MString("polyCone") ||
			_type == MString("polyPlane") ||
			_type == MString("polyBevel") ||
			_type == MString("polyBevel3") ||
			_type == MString("polyCollapseF") ||
			_type == MString("polyCollapseEdge"));
}

bool MayaUtils::doesItRequireConnections(MString& _type)
{
	return (_type == MString("polyTweak") ||
			_type == MString("polySplitRing") ||
			_type == MString("polyExtrudeFace") ||
			_type == MString("polyBevel") ||
			_type == MString("polyBevel3"));
}

PolyType MayaUtils::getPolyType(json& geoNode, MStatus& status)
{
	std::string type = geoNode["type"];
	status = MStatus::kSuccess;

	if (type.compare("polyCube") == 0)
	{
		return PolyType::CUBE;
	}

	if (type.compare("polyPipe") == 0)
	{
		return PolyType::PIPE;
	}

	if (type.compare("polySphere") == 0)
	{
		return PolyType::SPHERE;
	}

	if (type.compare("polyPyramid") == 0)
	{
		return PolyType::PYRAMID;
	}

	if (type.compare("polyCylinder") == 0)
	{
		return PolyType::CYLINDER;
	}

	if (type.compare("polyTorus") == 0)
	{
		return PolyType::TORUS;
	}

	if (type.compare("polyCone") == 0)
	{
		return PolyType::CONE;
	}

	if (type.compare("polyPlane") == 0)
	{
		return PolyType::PLANE;
	}

	status = MStatus::kFailure;
	return PolyType::CUBE;
}

MStatus MayaUtils::getNodeObjectFromUUID(MString& uuid, MObject& _node)
{
	// get node
	MSelectionList sList;
	MUuid ID(uuid);
	sList.add(ID);
	return sList.getDependNode(0, _node);
}

bool MayaUtils::doesItExist(std::string& _id)
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