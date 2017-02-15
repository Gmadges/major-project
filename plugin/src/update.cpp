#include "update.h"
#include "hackPrint.h"
#include "messaging.h"

#include "genericMessage.h"

Update::Update()
	:
	pMessenger(new Messaging("8080"))
{
}

Update::~Update()
{
}

void* Update::creator()
{
	return new Update;
}

MStatus	Update::doIt(const MArgList& args)
{
	MStatus status = MStatus::kSuccess;

	// ask the server for any update
	GenericMessage data;
	
	// if false then we couldnt connect to server
	if (!pMessenger->requestData(data)) return MStatus::kFailure;
	
	// is there actually anything?
	if (data.getNodeType().empty())
	{
		HackPrint::print("Nothing to update");
		return status;
	}

	// create a node of same type?

	// set all the write attributes

	// and add it to the DAG

	return status;
}

