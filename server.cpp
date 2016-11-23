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
	bool success = true;
	try
	{
		//auto parsed_route = router_.match(req->method(), req->uri().path());
		auto parsed_route = router_.match(http::method::code(req->method().to_string()), req->url().to_string());
		req->set_params(std::move(parsed_route.parsed_values));
		parsed_route.job(req, res);
	}
	catch (const std::exception& err)
	{
		printf("<Server> Router_error: %s - Responding with 404.\n", err.what());
		res->set_status(404, true);
		success = false;
	}

	return success;
}