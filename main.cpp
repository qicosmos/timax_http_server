#include <iostream>
#include <string>
#include "server.hpp"

int main()
{
	using namespace std::string_literals;
	using namespace timax;
	server_t server(9000, 4);
	http_router router;

	router.on_get("/", [](auto , auto res) 
	{
		res->add_body("<html><body><h1>Simple example</h1></body></html>"s);
	});

	router.on_get("/users/:id(\\d+)", [](auto req, auto res) 
	{
		auto id = req->params().at("id");

		if (id == "42")
		{
			res->add_body("hello timax server");
		}
		else
		{
			res->set_status(413, true);
		}
	});

	router.on_post("/users", [](auto req, auto res) 
	{
		res->add_body("hello world");
	});

	server.set_router(router).start();
	
	std::string str;
	std::cin >> str;
}