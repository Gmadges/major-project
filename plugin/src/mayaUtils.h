#ifndef MAYAUTILS_H
#define MAYAUTILS_H

#include <maya/MString.h>


class MayaUtils 
{
public:
	MayaUtils();
	~MayaUtils();

	static bool isValidNodeType(MString& _type);
};

#endif
