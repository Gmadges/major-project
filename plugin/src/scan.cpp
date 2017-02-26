#include "scan.h"

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MString.h>
#include <maya/MFnMesh.h>
#include <maya/MArgList.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MColor.h>
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

		dagPath.extendToShape();
		MObject meshNodeShape = dagPath.node();
		MFnDependencyNode depNodeFn(meshNodeShape);

		// Tweaks exist only if the multi "pnts" attribute contains
		// plugs that contain non-zero tweak values. Use false,
		// until proven true search pattern.

		MPlug tweakPlug = depNodeFn.findPlug("pnts");
		if (!tweakPlug.isNull())
		{
			// ASSERT : tweakPlug should be an array plug 
			//MAssert(tweakPlug.isArray(), "tweakPlug.isArray()");
			MPlug tweak;
			MFloatVector tweakData;
			int i;
			int numElements = tweakPlug.numElements();
			for (i = 0; i < numElements; i++)
			{
				tweak = tweakPlug.elementByPhysicalIndex(i, &status);
				if (status == MS::kSuccess && !tweak.isNull())
				{
					fHasTweaks = true;
					break;
				}
			}
		}

		MFnMesh mesh(meshNodeShape, &status);

		if (!status) {
			status.perror("MFnMesh:constructor");
			continue;
		}

		HackPrint::print("Shape: ");
		HackPrint::print(mesh.name());
		HackPrint::print("history: ");

		// If the inMesh is connected, we have history
		MPlug inMeshPlug = depNodeFn.findPlug("inMesh");
		if (inMeshPlug.isConnected())
		{
			MPlugArray tempPlugArray;
			inMeshPlug.connectedTo(tempPlugArray, true, false);
			MPlug upstreamNodeSrcPlug = tempPlugArray[0];
			MFnDependencyNode upstreamNode(upstreamNodeSrcPlug.node());

			HackPrint::print(upstreamNode.typeName());
			HackPrint::print(upstreamNode.name());
			findHistory(upstreamNode, mesh);
		}

		if (fHasTweaks) HackPrint::print("tweaks: true");
		else HackPrint::print("tweaks: false");

	}
	return MS::kSuccess;
}

void Scan::findHistory(MFnDependencyNode & node, MFnMesh & mesh)
{
	// check and send data about this node
	if (node.typeName() == MString("polySplitRing"))
	{
		HackPrint::print("we have found a polysplit lets send it");
		sendNode(node, mesh);
	}

	// need a niver wya of doing this with types over strings.
	if (node.typeName() == MString("polyCube"))
	{
		HackPrint::print("Found a cube, what do?");
		sendNode(node, mesh);
	}

	// now lets see if it has a parent

	// If the inpuPolymesh is connected, we have history
	MPlug inMeshPlug = node.findPlug("inputPolymesh");

	if (inMeshPlug.isConnected())
	{
		MPlugArray tempPlugArray;
		inMeshPlug.connectedTo(tempPlugArray, true, false);

		// Only one connection should exist on meshNodeShape.inMesh!
		MPlug upstreamNodeSrcPlug = tempPlugArray[0];

		MFnDependencyNode upstreamNode(upstreamNodeSrcPlug.node());
		
		HackPrint::print("----------");
		HackPrint::print(upstreamNode.typeName());
		HackPrint::print(upstreamNode.name());

		findHistory(upstreamNode, mesh);
	}
}

void Scan::sendNode(MFnDependencyNode & node, MFnMesh & mesh)
{
	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	unsigned int numAttrib = node.attributeCount();
	MStatus status;

	attribMap nodeAttribs;

	for (unsigned int i = 0; i < numAttrib; i++)
	{
		MFnAttribute attrib(node.attribute(i));

		MPlug plug = node.findPlug(attrib.shortName().asChar(), status);
		
		if (status != MStatus::kSuccess) continue;

		attribMap values;
		if (getAttribFromPlug(plug, values) == MStatus::kSuccess)
		{
			nodeAttribs.insert(values.begin(), values.end());
		}
	}

	if (nodeAttribs.empty()) return;

	GenericMessage msg;
	msg.setMeshName(std::string(mesh.name().asChar()));
	msg.setNodeName(std::string(node.name().asChar()));
	msg.setNodeType(node.typeName().asChar());
	msg.setRequestType(SCENE_UPDATE);

	msg.setAttribs(nodeAttribs);
	
	if (pMessaging->sendUpdate(msg))
	{
		HackPrint::print("Update sent Succesfully");
		return;
	}

	HackPrint::print("Cannot send to Server!");
}

MStatus Scan::getAttribFromPlug(MPlug& _plug, attribMap& _attribs)
{
	std::string attribName = _plug.partialName().asChar();
	
	if (_plug.isArray())
	{
		HackPrint::print(_plug.name());

		for (unsigned int i = 0; i<_plug.numElements(); ++i)
		{
			// get the MPlug for the i'th array element
			MPlug elemPlug = _plug.elementByPhysicalIndex(i);

			HackPrint::print(elemPlug.name());

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
			//HackPrint::print(childPlug.name());

			// get values
			// and store too
			attribMap values;
			if (getAttribFromPlug(childPlug, values) == MStatus::kSuccess)
			{
				_attribs.insert(values.begin(), values.end());
			}
		}

		// not sure what i should be doing with zone.
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