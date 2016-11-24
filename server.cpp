#include "server.hpp"

using namespace timax;
void server_t::do_accept()
{
	auto new_connection = std::make_shared<connection>(this, ios_pool_.get_io_service());
	acceptor_.async_accept(new_connection->socket(), [this, new_connection](boost::system::error_code const& error)
	{
		if (!error)
		{
			new_connection->start();
		}
		else
		{
			// TODO log error
		}

		do_accept();
	});
}

bool server_t::process_route(request_t* req, response_t* res)
{
	auto parsed_route = router_.match(http::method::code(req->method().to_string()), req->url().to_string());
	if (parsed_route.empty())
		return false;

	req->set_params(std::move(parsed_route.parsed_values));
	try
	{
		parsed_route.job(req, res);
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return false;
	}
	
	return true;
}