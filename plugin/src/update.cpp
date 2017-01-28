#include "update.h"

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

Update::Update()
{
}

Update::~Update()
{
}

MStatus Update::doScan()
{
	MItDag::TraversalType	traversalType = MItDag::kDepthFirst;

	MStatus status;

	MItDag dagIterator(traversalType, MFn::kMesh, &status);

	if (!status) {
		status.perror("MItDag constructor");
		return status;
	}

	MString resultString;
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

		// If the inMesh is connected, we have history
		MString historyString;
		MPlug inMeshPlug = depNodeFn.findPlug("inMesh");
		if (inMeshPlug.isConnected())
		{
			MPlugArray tempPlugArray;
			inMeshPlug.connectedTo(tempPlugArray, true, false);
			MPlug upstreamNodeSrcPlug = tempPlugArray[0];
			MFnDependencyNode upstreamNode(upstreamNodeSrcPlug.node());

			historyString += "\n";
			historyString += upstreamNode.typeName();
			historyString += " | ";
			historyString += upstreamNode.name();
			findHistory(historyString, upstreamNode);
		}

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

		resultString += ("\n Shape: " + mesh.name());
		resultString += "\n history: ";
		resultString += historyString;
		if (fHasTweaks) resultString += ("\n tweaks: true");
		else resultString += ("\n tweaks: false");

	}
	std::cout << resultString << std::endl;
	return MS::kSuccess;
}

void Update::findHistory(MString & string, MFnDependencyNode & node)
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

		// testing strings
		string += "\n";
		string += upstreamNode.typeName();
		string += " | ";
		string += upstreamNode.name();

		findHistory(string, upstreamNode);
	}
}
