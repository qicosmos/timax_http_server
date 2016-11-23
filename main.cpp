#include <iostream>
#include <string>
#include "server.hpp"

int main()
{
	using namespace timax;
	server_t server(9000, 4);
	server.start();
	
	std::string str;
	std::cin >> str;
}