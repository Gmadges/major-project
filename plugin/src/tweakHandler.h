#ifndef TWEAKHANDLER_H
#define TWEAKHANDLER_H

#include "maya/MStatus.h"
#include "maya/MDagPath.h"

class TweakHandler
{
public:
	TweakHandler();
	~TweakHandler();

	bool hasTweaks(MDagPath & meshDAGPath);
};

#endif

