#include "sendRegister.h"

#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MString.h>
#include <maya/MFnMesh.h>
#include <maya/MArgList.h>
#include <maya/MPoint.h>
#include <maya/MIOStream.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnAttribute.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MUuid.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>

#include "messaging.h"
#include "tweakHandler.h"
#include "hackPrint.h"
#include "testTypes.h"

#include "callbackHandler.h"

SendRegister::SendRegister()
	:
	SendAbstract()
{
}

SendRegister::~SendRegister()
{
}

void* SendRegister::creator()
{
	return new SendRegister;
}

MStatus	SendRegister::doIt(const MArgList& args)
{
	MStatus status;

	// reset socket
	MString addr;
	int port;
	status = getArgs(args, addr, port);
	if (status != MStatus::kSuccess)
	{
		HackPrint::print("no input values specified");
		return status;
	}

	pMessaging->resetSocket(std::string(addr.asChar()), port);

	MSelectionList selList;
	MGlobal::getActiveSelectionList(selList);
	MItSelectionList selListIter(selList);
	selListIter.setFilter(MFn::kMesh);

	// check any meshes are actually selected
	if (selListIter.isDone())
	{
		HackPrint::print("no mesh selected!");
		return MStatus::kFailure;
	}

	for (; !selListIter.isDone(); selListIter.next())
	{

		MDagPath dagPath;

		status = selListIter.getDagPath(dagPath);

		if (!status) {
			status.perror("MItDag::getPath");
			continue;
		}
		MFnDagNode dagNode(dagPath, &status);
		if (!status) {
			status.perror("MFnDagNode constructor");
			continue;
		}

		// check for tweaks
		if (pTweaksHandler->hasTweaks(dagPath))
		{
			MObject tweakNode;
			HackPrint::print("we got tweaks");
			if (pTweaksHandler->createPolyTweakNode(dagPath, tweakNode) == MStatus::kSuccess)
			{
				HackPrint::print("created a node ting");
				dagPath.extendToShape();
				if (pTweaksHandler->connectTweakNodes(tweakNode, dagPath.node()) == MStatus::kSuccess)
				{
					HackPrint::print("connected");
				}
			}
		}

		// turn tweaks into a node before sending
		if (sendMesh(dagPath) != MStatus::kSuccess) return MStatus::kFailure;

		// only gonna handle mesh for now
		break;
	}
	return MS::kSuccess;
}