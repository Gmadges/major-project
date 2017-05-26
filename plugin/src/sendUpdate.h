#ifndef SENDUPDATE_H
#define SENDUPDATE_H

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MSyntax.h>

#include <memory>
#include "json.h"

#include "sendAbstract.h"

class Messaging;

class SendUpdate : public SendAbstract
{
public:
	SendUpdate();
	~SendUpdate();

	// have to have a creator
	static void* creator();
	virtual MStatus	doIt(const MArgList&) override;

private:
	bool isNodeFromRegisteredMesh(MObject& _depNode);

private:

	void processNewNodes(std::vector<json>& nodeList);
	void processEditedNodes(std::vector<json>& nodeList);
	void processDeletedNodes(std::vector<json>& nodeList);
};

#endif