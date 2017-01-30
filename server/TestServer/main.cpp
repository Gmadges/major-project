#include <zmq.hpp>
#include "server.h"

int main() 
{	
	zmq::context_t context(1);
	Server server(context);
	server.run();
	return 0; 
}