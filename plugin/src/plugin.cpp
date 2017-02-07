////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
// 
// Produces the MEL command "ScanSend".
//
// This plug-in demonstrates walking the DAG using the DAG iterator class.
// 
// To use it:
//	(1) Create a number of objects anywhere in the scene.
//	(2) Execute the command "ScanSend". This will traverse the DAG printing
//		information about each node it finds to the window from which you
//		started Maya.
//
// The command accepts several flags:
//
//	-b/-breadthFirst  : Perform breadth first search 
//	-d/-depthFirst    : Perform depth first search 
//
////////////////////////////////////////////////////////////////////////

#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnPlugin.h>
#include <maya/MArgList.h>

#include <memory>
#include "scan.h"
#include "update.h"

class ScanSend : public MPxCommand
{
public:
	ScanSend();
	virtual	~ScanSend();
	static void* creator();
	virtual MStatus	doIt(const MArgList&);

private:
	std::unique_ptr<Scan> pScanner;
};

ScanSend::ScanSend()
	:
	pScanner(new Scan())
{
}

ScanSend::~ScanSend() {}

void* ScanSend::creator()
{
	return new ScanSend;
}

MStatus	ScanSend::doIt(const MArgList& args)
{
	return pScanner->doScan();
};

///////////////////////////////////////////////////////////// update

class ReceiveUpdate : public MPxCommand
{
public:
	ReceiveUpdate();
	virtual	~ReceiveUpdate();
	static void* creator();
	virtual MStatus	doIt(const MArgList&);

private:
	std::unique_ptr<Update> pUpdater;
};

ReceiveUpdate::ReceiveUpdate()
	:
	pUpdater(new Update())
{
}

ReceiveUpdate::~ReceiveUpdate() {}

void* ReceiveUpdate::creator()
{
	return new ReceiveUpdate;
}

MStatus	ReceiveUpdate::doIt(const MArgList& args)
{
	pUpdater->showUpdate();

	return MStatus::kSuccess;
};

///////////////////////////////////////////////////////////// initialise

MStatus initializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");
	status = plugin.registerCommand("ScanSend", ScanSend::creator);
	if (!status)
		status.perror("registerCommand");

	status = plugin.registerCommand("ReceiveUpdate", ReceiveUpdate::creator);
	if (!status)
		status.perror("registerCommand");

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj);
	status = plugin.deregisterCommand("ScanSend");
	if (!status)
		status.perror("deregisterCommand");
	status = plugin.deregisterCommand("ReceiveUpdate");
	if (!status)
		status.perror("deregisterCommand");


	return status;
}