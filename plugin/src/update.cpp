#include "update.h"
#include "subscriber.h"

Update::Update()
	:
	pSubscriber(new Subscriber("9000"))
{
}

Update::~Update()
{
}
