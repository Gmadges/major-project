#ifndef UPDATE_H
#define UPDATE_H

#include <maya/MStatus.h>

class Update
{
public:
	Update();
	~Update();

	MStatus doScan();
};

#endif