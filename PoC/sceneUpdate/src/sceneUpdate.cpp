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

class SceneUpdate : public MPxCommand
{
public:
	SceneUpdate() {};
	virtual	~SceneUpdate();
	static void* creator();
	virtual MStatus	doIt(const MArgList&);

private:
	
	MStatus doUpdate(const MItDag::TraversalType traversalType,
					MFn::Type filter);
};

SceneUpdate::~SceneUpdate() {}

void* SceneUpdate::creator()
{
	return new SceneUpdate;
}

MStatus	SceneUpdate::doIt(const MArgList& args)
{
	MItDag::TraversalType	traversalType = MItDag::kDepthFirst;
	MFn::Type				filter = MFn::kInvalid;

	return doUpdate(traversalType, filter);
};

MStatus SceneUpdate::doUpdate(const MItDag::TraversalType traversalType,
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
	status = plugin.registerCommand("sceneUpdate", SceneUpdate::creator);
	if (!status)
		status.perror("registerCommand");

	return status;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus status;

	MFnPlugin plugin(obj);
	status = plugin.deregisterCommand("sceneUpdate");
	if (!status)
		status.perror("deregisterCommand");

	return status;
}
