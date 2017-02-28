#include "server.h"
#include <iostream>

int main(int argc, char* argv[])
{	
	int port = 8080;

	if (argc >= 2)
	{
		try {
			port = std::stoi(argv[1]);
		}
		catch (std::invalid_argument&) {
			std::cerr << "Invalid number " << argv[1] << '\n';
		}
	}

	std::cout << "Serving on port: " << port << " ..." <<std::endl;

	Server server(port);
	return server.run();
}