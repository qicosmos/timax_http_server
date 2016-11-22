#include <iostream>
#include <string>
#include "server.hpp"
#include "http_router.hpp"

int main()
{
	timax::http_router router;
	router.match(timax::http::DEL, "/");

	using namespace timax;
	server_t server(9000, 4);
	server.start();
	
	std::string str;
	std::cin >> str;
}