#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnPlugin.h>

#include <memory>
#include "register.h"
#include "update.h"
#include "info.h"
#include "send.h"

// initialise our plugin and commands

MStatus initializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj, PLUGIN_COMPANY, "5.0", "Any");

	status = plugin.registerCommand("RegisterMesh", Register::creator, Register::newSyntax);
	if (!status)
	{
		status.perror("registerCommand");
	}

	status = plugin.registerCommand("sendUpdates", Send::creator, Send::newSyntax);
	if (!status)
	{
		status.perror("registerCommand");
	}

	status = plugin.registerCommand("ReceiveUpdate", Update::creator, Update::newSyntax);
	if (!status)
	{
		status.perror("registerCommand");
	}

	status = plugin.registerCommand("getInfo", Info::creator, Info::newSyntax);
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

	status = plugin.deregisterCommand("RegisterMesh");
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand("sendUpdates");
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand("ReceiveUpdate");
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	status = plugin.deregisterCommand("getInfo");
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	return status;
}
