////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
// 
// Produces the MEL command "scanDag".
//
// This plug-in demonstrates walking the DAG using the DAG iterator class.
// 
// To use it:
//	(1) Create a number of objects anywhere in the scene.
//	(2) Execute the command "scanDag". This will traverse the DAG printing
//		information about each node it finds to the window from which you
//		started Maya.
//
// The command accepts several flags:
//
//	-b/-breadthFirst  : Perform breadth first search 
//	-d/-depthFirst    : Perform depth first search 
//
////////////////////////////////////////////////////////////////////////

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MArgList.h>
#include <maya/MFnCamera.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MFnLight.h>
#include <maya/MColor.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MIOStream.h>

class scanDag : public MPxCommand
{
public:
	scanDag() {};
	virtual	~scanDag();
	static void* creator();
	virtual MStatus	doIt(const MArgList&);

private:

	MStatus parseArgs(const MArgList& args,
						MItDag::TraversalType&	traversalType,
						MFn::Type& filter);
	
	MStatus doScan(const MItDag::TraversalType traversalType,
					MFn::Type filter);
};

scanDag::~scanDag() {}

void* scanDag::creator()
{
	return new scanDag;
}

MStatus	scanDag::doIt(const MArgList& args)
{
	MItDag::TraversalType	traversalType = MItDag::kDepthFirst;
	MFn::Type				filter = MFn::kInvalid;
	MStatus					status;

	status = parseArgs(args, traversalType, filter);
	if (!status)
		return status;

	return doScan(traversalType, filter);
};

MStatus scanDag::parseArgs(const MArgList& args,
	MItDag::TraversalType& traversalType,
	MFn::Type& filter)
{
	MStatus     	stat;
	MString     	arg;
	const MString	breadthFlag("-b");
	const MString	breadthFlagLong("-breadthFirst");
	const MString	depthFlag("-d");
	const MString	depthFlagLong("-depthFirst");

	// Parse the arguments.
	for (unsigned int i = 0; i < args.length(); i++) {
		arg = args.asString(i, &stat);
		if (!stat)
			continue;

		if (arg == breadthFlag || arg == breadthFlagLong)
			traversalType = MItDag::kBreadthFirst;
		else if (arg == depthFlag || arg == depthFlagLong)
			traversalType = MItDag::kDepthFirst;
		else {
			arg += ": unknown argument";
			displayError(arg);
			return MS::kFailure;
		}
	}
	return stat;
}

MStatus scanDag::doScan(const MItDag::TraversalType traversalType,
	MFn::Type filter)
{
	MStatus status;

	MItDag dagIterator(traversalType, filter, &status);

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

		if (dagPath.hasFn(MFn::kMesh)) {

			MFnMesh mesh(dagPath, &status);

			if (!status) {
				status.perror("MFnMesh:constructor");
				continue;
			}
			resultString += "found mesh!\n";
			resultString += ("\n" + mesh.name());
		}
	}
	setResult(resultString);
	return MS::kSuccess;
}

MStatus initializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");
	status = plugin.registerCommand("scanDag", scanDag::creator);
	if (!status)
		status.perror("registerCommand");

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj);
	status = plugin.deregisterCommand("scanDag");
	if (!status)
		status.perror("deregisterCommand");

	return status;
}
