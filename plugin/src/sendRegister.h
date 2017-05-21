#ifndef SENDREGISTER_H
#define SENDREGISTER_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnMesh.h>
#include <maya/MPlug.h>
#include <maya/MSyntax.h>

#include <memory>
#include "json.h"
#include "sendAbstract.h"

class SendRegister : public SendAbstract
{
public:
	SendRegister();
	~SendRegister();

	static void* creator();
	virtual MStatus	doIt(const MArgList&) override;

private:
	// send the whole mesh and registers all callbacks
	MStatus registerAndSendMesh(MDagPath & meshDAGPath);

private:
	bool bTimerOn;
};

#endif