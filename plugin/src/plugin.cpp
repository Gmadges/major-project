#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnPlugin.h>
#include <maya/MArgList.h>

#include <memory>
#include "scan.h"
#include "update.h"

//TEST
#include "meshOpCmd.h"
#include "meshOpNode.h"

// initialise our plugin and commands

MStatus initializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj, PLUGIN_COMPANY, "5.0", "Any");

	status = plugin.registerCommand("ScanSend", Scan::creator);
	if (!status)
	{
		status.perror("registerCommand");
	}

	status = plugin.registerCommand("ReceiveUpdate", Update::creator);
	if (!status)
	{
		status.perror("registerCommand");
	}

	//TESTING
	status = plugin.registerCommand("meshOp", meshOp::creator);
	if (!status) 
	{
		status.perror("registerCommand");
		return status;
	}

	status = plugin.registerNode("meshOpNode",
									meshOpNode::id,
									meshOpNode::creator,
									meshOpNode::initialize);
	
	if (!status) 
	{
		status.perror("registerNode");
		return status;
	}

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj);

	status = plugin.deregisterCommand("ScanSend");
	if (!status)
	{
		status.perror("deregisterCommand");
	}
	status = plugin.deregisterCommand("ReceiveUpdate");
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	//TESTING
	status = plugin.deregisterCommand("meshOp");
	if (!status) 
	{
		status.perror("deregisterCommand");
		return status;
	}

	status = plugin.deregisterNode(meshOpNode::id);
	if (!status) 
	{
		status.perror("deregisterNode");
		return status;
	}

	return status;
}
