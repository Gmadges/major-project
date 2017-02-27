#ifndef UPDATE_H
#define UPDATE_H

#include "polyModifierCmd.h"

#include "maya/MArgList.h"
#include "maya/MFnDependencyNode.h"
#include <memory>

#include "genericMessage.h"

class Messaging;

class Update : public polyModifierCmd
{
public:
	Update();
	~Update();

	static void* creator();
	MStatus	doIt(const MArgList&);

private:
	void setNodeValues(MObject & node, GenericMessage & data);

private:
	std::unique_ptr<Messaging> pMessenger;

};

#endif