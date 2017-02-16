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

#include "genericMessage.h"

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
			findHistory(upstreamNode);
		}

		if (fHasTweaks) HackPrint::print("tweaks: true");
		else HackPrint::print("tweaks: false");

	}
	return MS::kSuccess;
}

void Scan::findHistory(MFnDependencyNode & node)
{
	// check and send data about this node
	if (node.typeName() == MString("polySplitRing"))
	{
		HackPrint::print("we have found a polysplit lets send it");
		sendPolySplitNode(node);
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

		findHistory(upstreamNode);
	}
}

void Scan::sendPolySplitNode(MFnDependencyNode & node)
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
		
		if (plug.isCompound())
		{
			//TODO
		}

		if (status == MStatus::kSuccess)
		{
			//MString value;
			float fValue;
			if (plug.getValue(fValue) == MStatus::kSuccess)
			{
				// maybe use some type defs to make this nicer;
				std::string attibName(attrib.shortName().asChar());
				attribType value(attibName, msgpack::object(fValue));
				nodeAttribs.insert(value);
				continue;
			}

			double dValue;
			if (plug.getValue(dValue) == MStatus::kSuccess)
			{
				std::string attibName(attrib.shortName().asChar());
				attribType value(attibName, msgpack::object(dValue));
				nodeAttribs.insert(value);
				continue;
			}

			MString sValue;
			if (plug.getValue(sValue) == MStatus::kSuccess)
			{
				std::string attibName(attrib.shortName().asChar());
				attribType value(attibName, msgpack::object(sValue.asChar()));
				nodeAttribs.insert(value);
				continue;
			}

			int iValue;
			if (plug.getValue(iValue) == MStatus::kSuccess)
			{
				std::string attibName(attrib.shortName().asChar());
				attribType value(attibName, msgpack::object(iValue));
				nodeAttribs.insert(value);
				continue;
			}

			bool bValue;
			if (plug.getValue(bValue) == MStatus::kSuccess)
			{
				std::string attibName(attrib.shortName().asChar());
				attribType value(attibName, msgpack::object(bValue));
				nodeAttribs.insert(value);
				continue;
			}
		}
	}

	if (nodeAttribs.empty()) return;

	GenericMessage msg;
	msg.setName(std::string(node.name().asChar()));
	msg.setNodeType(node.typeName().asChar());
	msg.setRequestType(SCENE_UPDATE);

	msg.setAttribs(nodeAttribs);

	// lets send the data if we have some
	HackPrint::print("send poly split data");
	
	if (pMessaging->sendUpdate(msg))
	{
		HackPrint::print("Update sent Succesfully");
		return;
	}

	HackPrint::print("Cannot send to Server!");
}