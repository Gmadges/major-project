#ifndef REGISTER_H
#define REGISTER_H

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

class Messaging;
class TweakHandler;

class Register : public MPxCommand
{
public:
	Register();
	~Register();

	static void* creator();
	static MSyntax newSyntax();
	virtual MStatus	doIt(const MArgList&);

private:

	MStatus getArgs(const MArgList& args, MString& address, int& port);
	void traverseHistory(MFnDependencyNode & node,std::vector<json>& nodeList);
	MStatus getGenericNode(MFnDependencyNode & _inNode, json& _outNode);
	MStatus getAttribFromPlug(MPlug& _plug, json& _attribs);
	MStatus sendMesh(MDagPath & meshNode);

	std::unique_ptr<Messaging> pMessaging;
	std::unique_ptr<TweakHandler> pTweaksHandler;
};

#endif