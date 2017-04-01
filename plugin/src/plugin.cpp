#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnPlugin.h>

#include <memory>
#include "sendRegister.h"
#include "requestMesh.h"
#include "info.h"
#include "sendUpdate.h"
#include "setServerCmd.h"

// initialise our plugin and commands

static const MString registerCmd = "RegisterMesh";
static const MString sendUpdateCmd = "SendUpdates";
static const MString RecieveUpdateCmd = "ReceiveUpdate";
static const MString getServerInfoCmd = "GetInfo";
static const MString setServerCmd = "SetServer";

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

	status = plugin.registerCommand(RecieveUpdateCmd, RequestMesh::creator, RequestMesh::newSyntax);
	if (!status)
	{
		status.perror("registerCommand");
	}

	status = plugin.registerCommand(getServerInfoCmd, Info::creator);
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

	status = plugin.deregisterCommand(registerCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand(sendUpdateCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand(RecieveUpdateCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand(getServerInfoCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand(setServerCmd);
	if (!status)
	{
		status.perror("deregisterCommand");
	}


	return status;
}
