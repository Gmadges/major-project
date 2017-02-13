#ifndef UPDATE_H
#define UPDATE_H

#include "maya/MArgList.h"
#include "maya/MPxCommand.h"

class Update : public MPxCommand
{
public:
	Update();
	~Update();

	static void* creator();
	MStatus	doIt(const MArgList&);

private:

};

#endif