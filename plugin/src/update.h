#ifndef UPDATE_H
#define UPDATE_H

#include "polyModifierCmd.h"

#include "maya/MArgList.h"
#include "maya/MFnDependencyNode.h"
#include <maya/MSyntax.h>
#include <memory>

#include "genericMessage.h"

class Messaging;

class Update : public polyModifierCmd
{
public:
	Update();
	~Update();

	static void* creator();
	static MSyntax Update::newSyntax();
	MStatus	doIt(const MArgList&);

private:
	MStatus getArgs(const MArgList& args, MString& address, int& port);
	void setNodeValues(MObject & node, GenericMessage & data);

private:
	std::unique_ptr<Messaging> pMessenger;

};

#endif