#include "update.h"

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MString.h>
#include <maya/MFnMesh.h>
#include <maya/MArgList.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MColor.h>
#include <maya/MIOStream.h>

Update::Update()
{
}

Update::~Update()
{
}

MStatus Update::doScan()
{
	MItDag::TraversalType	traversalType = MItDag::kDepthFirst;
	MFn::Type				filter = MFn::kInvalid;

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
	// cant call this
	//setResult(resultString);
	return MS::kSuccess;
}
