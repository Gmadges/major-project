#ifndef MAYAUTILS_H
#define MAYAUTILS_H

#include <maya/MString.h>
#include <maya/MObject.h>

class MayaUtils 
{
public:
	MayaUtils();
	~MayaUtils();

	static bool isValidNodeType(MString& _type);
	static MStatus getNodeObjectFromUUID(MString& uuid, MObject& _node);
	static bool doesItExist(std::string& id);
};

#endif
