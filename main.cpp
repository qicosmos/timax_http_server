#include <iostream>
#include <string>
#include "server.hpp"

int main()
{
	using namespace std::string_literals;
	using namespace timax;
	server_t server(9000);
	http_router router;

	//request_t request;
	//const std::string str1 = "GET / HTTP/1.0\r\n";
	//const std::string str2 = "GET / HTTP/1.0\r\nHost: 192.168.2.239:9000\r\n";
	//const std::string str3 = "GET / HTTP/1.0\r\nHost: 192.168.2.239:9000\r\nUser-Agent: ApacheBench/2.3\r\n";
	//const std::string str4 = "GET / HTTP/1.0\r\nHost: 192.168.2.239:9000\r\nUser-Agent: ApacheBench/2.3\r\nAccept: */*\r\n";
	//const std::string str5 = "GET / HTTP/1.0\r\nHost: 192.168.2.239:9000\r\nUser-Agent: ApacheBench/2.3\r\nAccept: */*\r\n\r\n";

	//int r = request.parse(str1, 0);
	//r = request.parse(str2, str1.size());
	//r = request.parse(str3, str2.size());
	//r = request.parse(str4, str3.size());
	//r = request.parse(str5, str4.size());

	router.on_get("/", [](auto , auto res) 
	{
		res->add_body("hello world"s);
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

	server.set_static_dir("./static").set_router(router).start();
	
	std::string str;
	std::cin >> str;
}