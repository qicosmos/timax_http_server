#pragma once
#include <boost/asio.hpp>
#include "request.hpp"

namespace timax
{
	class connection : public std::enable_shared_from_this<connection>
	{
	public:

		connection(boost::asio::io_service& ios) : socket_(ios), read_buf_(MAX_LEN)
		{
		}

		void start()
		{

		}

		void read_head()
		{
			auto self = this->shared_from_this();
			boost::asio::async_read_until(socket_, read_buf_, "\r\n\r\n", [this, self]
				(const boost::system::error_code& ec, std::size_t bytes_transferred)
			{
				if (ec != 0) 
				{
					//on_finish(ec);
					close();
					return;
				}

				//parser http header
				request_t request;
				
				int r = request.parse(boost::asio::buffer_cast<const char*>(read_buf_.data()), bytes_transferred);
				if (r < 0)
				{
					//on error
					close();
					return;
				}

				size_t body_len = request.body_length();
				if (body_len == 0)
					read_body(self, std::move(request));
				else
					read_body(self, std::move(request), body_len);
			});
		}

		void read_body(const std::shared_ptr<connection>& self, request_t request)
		{
			//wait for eof
			boost::asio::async_read(socket_, read_buf_, [this, self, req = std::move(request)]
				(const boost::system::error_code& ec, std::size_t bytes_transferred)
			{
				if (ec == boost::asio::error::eof)
				{
					shutdown_short_conneciton(req);
				}

				//on error
				close();
			});
		}

		void read_body(const std::shared_ptr<connection>& self, request_t request, size_t body_len)
		{
			//read http body
			boost::asio::async_read(socket_, read_buf_, [body_len](const boost::system::error_code &, size_t size) { return size = body_len; }, 
				[this, self, req = std::move(request)]
				(const boost::system::error_code& ec, std::size_t bytes_transferred)
			{
				if (ec)//on error
				{
					close();
					return;
				}

				//callback to user
				//todo

				//shutdown short connection
				shutdown_short_conneciton(req);
			});
		}

		void shutdown_short_conneciton(const request_t& request)
		{
			if (request.minor_version() == 0 && !request.has_keepalive_attr()) //short conneciton
			{
				shutdown(socket_);
				return;
			}

			if (request.minor_version() == 1 && request.has_close_attr())
			{
				shutdown(socket_);
				return;
			}
		}

		bool shutdown(boost::asio::ip::tcp::socket& s)
		{
			boost::system::error_code ignored_ec;
			s.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ignored_ec);

			return false;
		}

		void close()
		{
			socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			boost::system::error_code ignored_ec;
			socket_.close(ignored_ec);
		}

	private:
		boost::asio::ip::tcp::socket socket_;
		boost::asio::streambuf read_buf_;
		const int MAX_LEN = 8192;
	};
}

