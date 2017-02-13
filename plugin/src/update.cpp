#include "update.h"
#include "hackPrint.h"
#include "messaging.h"

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
	//HackPrint::print("We have recieved ");
	//HackPrint::print(std::to_string(pSubscriber->updates));
	//HackPrint::print("updates");

	// ask for update

	// create a node of same type?

	// set all the write attributes

	// and add it to the DAG

	return MStatus::kSuccess;
};

