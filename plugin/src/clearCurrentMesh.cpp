#include "clearCurrentMesh.h"

#include "callbackHandler.h"
#include <maya/MGlobal.h>

ClearCurrentMesh::ClearCurrentMesh()
{
}


ClearCurrentMesh::~ClearCurrentMesh()
{
}

void* ClearCurrentMesh::creator()
{
	return new ClearCurrentMesh;
}

MStatus ClearCurrentMesh::doIt(const MArgList& args)
{
	MStatus status;

	// send any updates we have
	MGlobal::executeCommand("SendUpdates");

	// clear all callbacks
	CallbackHandler::getInstance().clearCallbacks();

	// select this not as the current mesh
	CallbackHandler::getInstance().setCurrentRegisteredMesh("");

	return MS::kSuccess;
}
