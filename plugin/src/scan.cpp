#include "scan.h"

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

#include "messaging.h"
#include "hackPrint.h"

Scan::Scan()
	:
	pMessaging(new Messaging("8080"))
{
}

Scan::~Scan()
{
}

void* Scan::creator()
{
	return new Scan;
}

MStatus	Scan::doIt(const MArgList& args)
{
	MItDag::TraversalType	traversalType = MItDag::kDepthFirst;

	MStatus status;

	// filter to only meshes
	MItDag dagIterator(traversalType, MFn::kMesh, &status);

	if (!status) {
		status.perror("MItDag constructor");
		return status;
	}

	for (; !dagIterator.isDone(); dagIterator.next()) {

		MDagPath dagPath;

		status = dagIterator.getPath(dagPath);
		if (!status) {
			status.perror("MItDag::getPath");
			continue;
		}
		MFnDagNode dagNode(dagPath, &status);
		if (!status) {
			status.perror("MFnDagNode constructor");
			continue;
		}
		bool fHasTweaks = false;

		//// Tweaks exist only if the multi "pnts" attribute contains
		//// plugs that contain non-zero tweak values. Use false,
		//// until proven true search pattern.
		//MPlug tweakPlug = depNodeFn.findPlug("pnts");
		//if (!tweakPlug.isNull())
		//{
		//	// ASSERT : tweakPlug should be an array plug 
		//	//MAssert(tweakPlug.isArray(), "tweakPlug.isArray()");
		//	MPlug tweak;
		//	MFloatVector tweakData;
		//	int i;
		//	int numElements = tweakPlug.numElements();
		//	for (i = 0; i < numElements; i++)
		//	{
		//		tweak = tweakPlug.elementByPhysicalIndex(i, &status);
		//		if (status == MS::kSuccess && !tweak.isNull())
		//		{
		//			fHasTweaks = true;
		//			break;
		//		}
		//	}
		//}

		if (sendMesh(dagPath) != MStatus::kSuccess) return MStatus::kFailure;

		if (fHasTweaks) HackPrint::print("tweaks: true");
		else HackPrint::print("tweaks: false");
	}
	return MS::kSuccess;
}

MStatus Scan::sendMesh(MDagPath & meshDAGPath)
{
	MStatus status;
	GenericMesh meshMessage;
	std::vector<GenericNode> nodeList;

	meshDAGPath.extendToShape();
	MObject meshNodeShape = meshDAGPath.node();
	MFnDependencyNode depNodeFn(meshNodeShape);

	MFnMesh mesh(meshNodeShape, &status);

	if (!status) 
	{
		status.perror("MFnMesh:constructor");
		return status;
	}

	MFnDependencyNode transformNode(meshDAGPath.transform());
	GenericNode transNode;
	status = getGenericNode(transformNode, transNode);
	if (status != MStatus::kSuccess) return status;
	nodeList.push_back(transNode);

	traverseHistory(depNodeFn, nodeList);

	// should have atleast 3 nodes for a mesh
	// transform, shape and mesh
	if (nodeList.size() < 3) return MStatus::kFailure;

	meshMessage.setMeshName(std::string(mesh.name().asChar()));
	meshMessage.setRequestType(SCENE_UPDATE);

	// hardcode for now
	meshMessage.setMeshType(CUBE);

	meshMessage.setNodes(nodeList);

	HackPrint::print("sending " + mesh.name());
	if (pMessaging->sendUpdate(meshMessage))
	{
		HackPrint::print("mesh sent succesfully");
		return MStatus::kSuccess;
	}

	HackPrint::print("unable to send");
	return MStatus::kFailure;
}

void Scan::traverseHistory(MFnDependencyNode & node, std::vector<GenericNode>& nodeList)
{
	HackPrint::print(node.name());
	HackPrint::print(node.typeName());
	
	if (node.typeName() == MString("transform") ||
		node.typeName() == MString("mesh") ||
		//node.typeName() == MString("polySplitRing") ||
		node.typeName() == MString("polyCube"))
	{
		GenericNode genNode;
		if (getGenericNode(node, genNode) == MStatus::kSuccess)
		{
			nodeList.push_back(genNode);
		}
	}

	// now lets see if it has a parent

	// If the inpuPolymesh is connected, we have history
	MStatus status;
	MPlug inMeshPlug;
	inMeshPlug = node.findPlug("inputPolymesh", &status);
	
	// if it doesnt have that plug try this one
	if (status != MStatus::kSuccess)
	{
		inMeshPlug = node.findPlug("inMesh");
	}

	if (inMeshPlug.isConnected())
	{
		MPlugArray tempPlugArray;
		inMeshPlug.connectedTo(tempPlugArray, true, false);
		// Only one connection should exist on meshNodeShape.inMesh!
		MPlug upstreamNodeSrcPlug = tempPlugArray[0];
		MFnDependencyNode upstreamNode(upstreamNodeSrcPlug.node());

		traverseHistory(upstreamNode, nodeList);
	}
}

MStatus Scan::getGenericNode(MFnDependencyNode & _inNode, GenericNode& _outNode)
{
	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	unsigned int numAttrib = _inNode.attributeCount();
	MStatus status;

	attribMap nodeAttribs;

	for (unsigned int i = 0; i < numAttrib; i++)
	{
		MFnAttribute attrib(_inNode.attribute(i));

		MPlug plug = _inNode.findPlug(attrib.shortName().asChar(), &status);
		
		if (status != MStatus::kSuccess) continue;

		attribMap values;
		if (getAttribFromPlug(plug, values) == MStatus::kSuccess)
		{
			nodeAttribs.insert(values.begin(), values.end());
		}
	}

	if (nodeAttribs.empty()) return MStatus::kFailure;

	_outNode.setNodeName(std::string(_inNode.name().asChar()));
	_outNode.setNodeType(_inNode.typeName().asChar());
	_outNode.setAttribs(nodeAttribs);

	return MStatus::kSuccess;
}

MStatus Scan::getAttribFromPlug(MPlug& _plug, attribMap& _attribs)
{
	std::string attribName = _plug.partialName().asChar();

	if (_plug.isArray())
	{
		for (unsigned int i = 0; i < _plug.numConnectedElements(); i++)
		{
			// get the MPlug for the i'th array element
			MPlug elemPlug = _plug.connectionByPhysicalIndex(i);

			attribMap values;
			if (getAttribFromPlug(elemPlug, values) == MStatus::kSuccess)
			{
				_attribs.insert(values.begin(), values.end());
			}
		}

		// need to ad somethinf with a bit more info
		_attribs.insert(attribType(attribName, msgpack::object()));
		return MStatus::kSuccess;
	}

	if (_plug.isCompound())
	{
		// if the plug is a compound then it has a number of children plugs we need to grab
		unsigned int numChild = _plug.numChildren();

		for(unsigned int i = 0; i < numChild; ++i)
		{
			MPlug childPlug = _plug.child(i);

			// get values
			// and store too
			attribMap values;
			if (getAttribFromPlug(childPlug, values) == MStatus::kSuccess)
			{
				_attribs.insert(values.begin(), values.end());
			}
		}

		// should put something about this
		_attribs.insert(attribType(attribName, msgpack::object()));
		return MStatus::kSuccess;
	}

    //MString value;
    float fValue;
    if (_plug.getValue(fValue) == MStatus::kSuccess)
    {
		_attribs.insert(attribType(attribName, msgpack::object(fValue)));
		return MStatus::kSuccess;
    }

    double dValue;
    if (_plug.getValue(dValue) == MStatus::kSuccess)
    {
		_attribs.insert(attribType(attribName, msgpack::object(dValue)));
		return MStatus::kSuccess;
    }

    MString sValue;
    if (_plug.getValue(sValue) == MStatus::kSuccess)
    {
		_attribs.insert(attribType(attribName, msgpack::object(sValue.asChar())));
		return MStatus::kSuccess;
    }

    int iValue;
    if (_plug.getValue(iValue) == MStatus::kSuccess)
    {
		_attribs.insert(attribType(attribName, msgpack::object(iValue)));
		return MStatus::kSuccess;
    }

    bool bValue;
    if (_plug.getValue(bValue) == MStatus::kSuccess)
    {
		_attribs.insert(attribType(attribName, msgpack::object(bValue)));
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