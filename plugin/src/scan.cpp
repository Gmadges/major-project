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

// hacky method to easily print
#include <maya/MGlobal.h>
void print(const char * text)
{
	MGlobal::displayInfo(text);
}
void print(MString& text)
{
	MGlobal::displayInfo(text);
}

#include "messaging.h"

Scan::Scan()
	:
	pMessaging(new Messaging("8080"))
{
}

Scan::~Scan()
{
}

MStatus Scan::doScan()
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

		print("Shape: ");
		print(mesh.name());
		print("history: ");

		// If the inMesh is connected, we have history
		MPlug inMeshPlug = depNodeFn.findPlug("inMesh");
		if (inMeshPlug.isConnected())
		{
			MPlugArray tempPlugArray;
			inMeshPlug.connectedTo(tempPlugArray, true, false);
			MPlug upstreamNodeSrcPlug = tempPlugArray[0];
			MFnDependencyNode upstreamNode(upstreamNodeSrcPlug.node());

			print(upstreamNode.typeName());
			print(upstreamNode.name());
			findHistory(upstreamNode);
		}

		if (fHasTweaks) print("tweaks: true");
		else print("tweaks: false");

	}
	return MS::kSuccess;
}

void Scan::findHistory(MFnDependencyNode & node)
{
	// If the inpuPolymesh is connected, we have history
	MPlug inMeshPlug = node.findPlug("inputPolymesh");

	if (inMeshPlug.isConnected())
	{
		MPlugArray tempPlugArray;
		inMeshPlug.connectedTo(tempPlugArray, true, false);

		// Only one connection should exist on meshNodeShape.inMesh!
		MPlug upstreamNodeSrcPlug = tempPlugArray[0];

		MFnDependencyNode upstreamNode(upstreamNodeSrcPlug.node());
		
		print("----------");
		print(upstreamNode.typeName());
		print(upstreamNode.name());

		if (upstreamNode.typeName() == MString("polySplitRing"))
		{
			print("we have found a polysplit lets send it");
			sendPolySplitNode(upstreamNode);
		}

		findHistory(upstreamNode);
	}
}

void Scan::sendPolySplitNode(MFnDependencyNode & node)
{
	// this shows us all attributes.
	// there are other ways of individually finding them using plugs
	unsigned int numAttrib = node.attributeCount();
	MStatus status;

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
				//object[attrib.shortName().asChar()] = fValue;
			}

			double dValue;
			if (plug.getValue(dValue) == MStatus::kSuccess)
			{
				//object[attrib.shortName().asChar()] = dValue;
			}

			MString sValue;
			if (plug.getValue(sValue) == MStatus::kSuccess)
			{
				//object[attrib.shortName().asChar()] = sValue.asChar();
			}

			int iValue;
			if (plug.getValue(iValue) == MStatus::kSuccess)
			{
				//object[attrib.shortName().asChar()] = iValue;
			}

			bool bValue;
			if (plug.getValue(bValue) == MStatus::kSuccess)
			{
				//object[attrib.shortName().asChar()] = bValue;
			}
		}
	}

	// lets send the data if we have some
	print("send poly split data");
	pMessaging->send();
}