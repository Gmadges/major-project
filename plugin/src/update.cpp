#include "update.h"
#include "subscriber.h"
#include "hackPrint.h"

Update::Update()
	:
	pSubscriber(new Subscriber("9000"))
{
}

Update::~Update()
{
}

void Update::showUpdate()
{
	HackPrint::print("We have recieved ");
	HackPrint::print(std::to_string(pSubscriber->updates));
	HackPrint::print("updates");
}
