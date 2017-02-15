#ifndef UPDATE_H
#define UPDATE_H

#include "maya/MArgList.h"
#include "maya/MPxCommand.h"
#include "maya/MFnDependencyNode.h"
#include <memory>

#include "genericMessage.h"

class Messaging;

class Update : public MPxCommand
{
public:
	Update();
	~Update();

	static void* creator();
	MStatus	doIt(const MArgList&);

private:
	void setNodeValues(MFnDependencyNode & node, GenericMessage & data);

private:
	std::unique_ptr<Messaging> pMessenger;

};

#endif