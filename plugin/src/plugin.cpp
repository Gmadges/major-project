#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnPlugin.h>

#include <memory>
#include "sendRegister.h"
#include "requestMesh.h"
#include "requestUpdate.h"
#include "sendUpdate.h"
#include "setServerCmd.h"
#include "clearCurrentMesh.h"
#include "callbackHandler.h"

// initialise our plugin and commands

static const MString registerCmd = "RegisterMesh";
static const MString sendUpdateCmd = "SendUpdates";
static const MString RequestMeshCmd = "RequestMesh";
static const MString RequestUpdateCmd = "RequestUpdate";
static const MString setServerCmd = "SetServer";
static const MString clearCurrentCmd = "ClearCurrentMesh";

MStatus initializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj, PLUGIN_COMPANY, "5.0", "Any");

	status = plugin.registerCommand(registerCmd, SendRegister::creator);
	if (!status)
	{
		status.perror("registerCommand");
	}

	status = plugin.registerCommand(sendUpdateCmd, SendUpdate::creator);
	if (!status)
	{
		status.perror("registerCommand");
	}

	status = plugin.registerCommand(clearCurrentCmd, ClearCurrentMesh::creator);
	if (!status)
	{
		status.perror("registerCommand");
	}

	status = plugin.registerCommand(RequestMeshCmd, RequestMesh::creator, RequestMesh::newSyntax);
	if (!status)
	{
		status.perror("registerCommand");
	}

	status = plugin.registerCommand(RequestUpdateCmd, RequestUpdate::creator, RequestUpdate::newSyntax);
	if (!status)
	{
		status.perror("registerCommand");
	}

	status = plugin.registerCommand(setServerCmd, SetServerCmd::creator, SetServerCmd::newSyntax);
	if (!status)
	{
		status.perror("registerCommand");
	}

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj);

	status = plugin.deregisterCommand(setServerCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand(RequestUpdateCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand(RequestMeshCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand(clearCurrentCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand(sendUpdateCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand(registerCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	// just making sure we get rid of the callbacks
	CallbackHandler::getInstance().clearCallbacks();

	return status;
}
