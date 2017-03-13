#include "tweakHandler.h"

#include "maya/MPlug.h"
#include "maya/MPlugArray.h"
#include "maya/MFloatVector.h"
#include "maya/MFnNumericData.h"
#include "maya/MFnDependencyNode.h"

TweakHandler::TweakHandler()
{
}


TweakHandler::~TweakHandler()
{
}

bool TweakHandler::hasTweaks(MDagPath& meshDAGPath)
{
	meshDAGPath.extendToShape();
	MFnDependencyNode depNodeFn(meshDAGPath.node());

	MStatus status;
	MPlug tweakPlug = depNodeFn.findPlug("pnts");
	if (!tweakPlug.isNull())
	{
		if (!tweakPlug.isArray()) return false;

		MPlug tweak;
		MFloatVector tweakData;
		int i;
		int numElements = tweakPlug.numElements();

		for (i = 0; i < numElements; i++)
		{
			tweak = tweakPlug.elementByPhysicalIndex(i, &status);
			if (status == MS::kSuccess && !tweak.isNull())
			{
				// get the values from plug
				MObject object;
				tweak.getValue(object);
				MFnNumericData numDataFn(object);
				numDataFn.getData(tweakData[0], tweakData[1], tweakData[2]);

				if (0 != tweakData.x ||
					0 != tweakData.y ||
					0 != tweakData.z)
				{
					return true;
				}
			}
		}
	}

	return false;
}
