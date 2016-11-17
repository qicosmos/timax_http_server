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
			read_head();
		}

		boost::asio::ip::tcp::socket& socket()
		{
			return socket_;
		}

	private:
		void read_head()
		{
			read_buf_.consume(read_buf_.size());//Çå¿Õbuf
			auto self = this->shared_from_this();
			boost::asio::async_read_until(socket_, read_buf_, "\r\n\r\n", [this, self]
				(const boost::system::error_code& ec, std::size_t bytes_transferred)
			{
				if (ec != 0) 
				{
					if (ec == boost::asio::error::eof)
					{
						std::cout << "client socket shutdown" << std::endl;
					}
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
				{
					if(request.has_keepalive_attr())
						send_response();

					if(!close_short_conneciton(request))
						read_head();
				}	
				else
				{
					read_body(self, std::move(request), body_len);
				}					
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
				//response
				//todo

				close_short_conneciton(req);
			});
		}

		bool close_short_conneciton(const request_t& request)
		{
			if (request.minor_version() == 0 && !request.has_keepalive_attr()) //short conneciton
			{
				close();
				return true;
			}

			if (request.minor_version() == 1 && request.has_close_attr())
			{
				close();
				return true;
			}

			return false;
		}

		void shutdown_send(boost::asio::ip::tcp::socket& s)
		{
			boost::system::error_code ignored_ec;
			s.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ignored_ec);
		}

		void close()
		{
			boost::system::error_code ignored_ec;
			socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			socket_.close(ignored_ec);
		}

		void send_response() 
		{
			const std::string str = "HTTP/1.1 200 OK\r\n"
				"Date: Thu, 17 Nov 2016 08:38:08 GMT\r\n"
				"Server: Apache\r\n"
				"Content-Length: 11\r\n"
				"Connection: Keep-Alive\r\n"
				"Content-Type: text/html\r\n"
				"\r\n"
				"hello world";
			boost::system::error_code ec;
			boost::asio::write(socket_, boost::asio::buffer(str), ec);
		}

	private:
		boost::asio::ip::tcp::socket socket_;
		boost::asio::streambuf read_buf_;
		char buf_[1];
		const int MAX_LEN = 8192;
		const static std::map<unsigned int, std::string> http_status_table;
	};

	const std::map<unsigned int, std::string>
		connection::http_status_table =
	{
		{ 200, "200 OK" },
		{ 404, "404 Not Found" },
		{ 413, "413 Request Entity Too Large" },
		{ 500, "500 Server Error" },
		{ 501, "501 Not Implemented" },
		{ 505, "505 HTTP Version Not Supported" }
	};
}

