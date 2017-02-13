#ifndef UPDATE_H
#define UPDATE_H

#include "maya/MArgList.h"
#include "maya/MPxCommand.h"
#include <memory>

class Messaging;

class Update : public MPxCommand
{
public:
	Update();
	~Update();

	static void* creator();
	MStatus	doIt(const MArgList&);

private:
	void requestInfo();

private:
	std::unique_ptr<Messaging> pMessenger;

};

#endif