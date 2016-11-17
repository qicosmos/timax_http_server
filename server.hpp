#pragma once
#include <memory>
#include <boost/asio.hpp>
#include "io_service_pool.hpp"
#include "connection.hpp"
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

	private:
		io_service_pool				ios_pool_;
		tcp::acceptor				acceptor_;
	};
}


