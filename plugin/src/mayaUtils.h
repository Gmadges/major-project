#ifndef MAYAUTILS_H
#define MAYAUTILS_H

#include <maya/MString.h>
#include <maya/MObject.h>
#include <maya/MFnDependencyNode.h>

class MayaUtils 
{
public:
	MayaUtils();
	~MayaUtils();

	static bool isValidNodeType(MString& _type);
	static MStatus getNodeObjectFromUUID(MString& uuid, MObject& _node);
	static bool doesItExist(std::string& id);
	static MStatus getIncomingNodeObject(MFnDependencyNode& node, MFnDependencyNode& incomingNode);
	static MStatus getOutgoingNodeObject(MFnDependencyNode& node, MFnDependencyNode& outgoingNode);
	static MPlug getInPlug(MFnDependencyNode& node, MStatus& status);
	static MPlug getOutPlug(MFnDependencyNode& node, MStatus &status);
};

#endif
