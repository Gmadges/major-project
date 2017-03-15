#ifndef TWEAKHANDLER_H
#define TWEAKHANDLER_H

#include "maya/MStatus.h"
#include "maya/MDagPath.h"
#include "maya/MDGModifier.h"
#include "maya/MPlug.h"
#include "maya/MFloatVector.h"

#include "json.h"
#include <vector>

class TweakHandler
{
public:
	TweakHandler();
	~TweakHandler();

	bool hasTweaks(MDagPath & meshDAGPath);
	// create a tweak node and removed tweaks from mesh
	MStatus createPolyTweakNode(MDagPath & meshDAGPath, MObject& tweakNode);
	MStatus connectTweakNodes(MObject& tweakNode, MObject& meshNode);

	// this method returns the tweak values in and array that we send
	MStatus setTweakPlugFromArray(MPlug& _plug, std::vector<json>& tweaks);

private:
	MFloatVector getFloat3FromPlug(MPlug& plug);

private:
	MDGModifier	fDGModifier;
};

#endif

