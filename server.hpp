#pragma once
#include <memory>
#include <boost/asio.hpp>
#include "io_service_pool.hpp"
#include "connection.hpp"
#include "http_router.hpp"
using namespace boost::asio::ip;

namespace timax
{
	class server_t
	{
	public:

		server_t(uint16_t port, size_t pool_size) : ios_pool_(pool_size),
			acceptor_(ios_pool_.get_io_service(), tcp::endpoint{ tcp::v4(), port })
		{
		}

		~server_t()
		{
			stop();
		}

		void start()
		{
			ios_pool_.start();
			do_accept();
		}

		void stop()
		{
			ios_pool_.stop();
		}

		http_router& router() noexcept 
		{
			return router_;
		}

	private:
		void do_accept()
		{
			auto new_connection = std::make_shared<connection>(ios_pool_.get_io_service());
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

		void process_route(request_t* req, response_t* res) 
		{
			try 
			{
				//auto parsed_route = router_.match(req->method(), req->uri().path());
				auto parsed_route = router_.match(http::method::code(req->method().to_string()), req->url().to_string());
				//req->set_params(parsed_route.parsed_values);
				parsed_route.job(req, res);
			}
			catch (const std::exception& err) 
			{
				printf("<Server> Router_error: %s - Responding with 404.\n", err.what());
				//res->send_response(send_code(http::Not_Found, true);
			}
		}

	private:
		io_service_pool				ios_pool_;
		tcp::acceptor				acceptor_;
		http_router					router_;
	};
}


