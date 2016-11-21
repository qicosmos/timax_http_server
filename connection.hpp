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

		~connection()
		{
			close();
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
			read_buf_.consume(read_buf_.size());//清空buf
			auto self = this->shared_from_this();
			boost::asio::async_read_until(socket_, read_buf_, "\r\n\r\n", [this, self]
				(const boost::system::error_code& ec, std::size_t bytes_transferred)
			{
				size_t statas_code = 200;
				if (ec != 0) 
				{
					if (ec == boost::asio::error::eof)
					{
						std::cout << "client socket shutdown" << std::endl;
					}
					else if (ec == boost::asio::error::not_found)
					{
						statas_code = 413;
						response_and_close(statas_code, self);
						return;
					}
					close();
					return;
				}

				request_t request;
				int r = request.parse(boost::asio::buffer_cast<const char*>(read_buf_.data()), bytes_transferred);
				if (r < 0)
				{
					statas_code = 400;
					response_and_close(statas_code, self);
					return;
				}

				bool need_close = need_close_conneciton(request);
				size_t body_len = request.body_length();
				if (body_len == 0)
				{
					if (request.has_keepalive_attr())
					{
						response_and_read(statas_code, need_close, self);
					}
					else
					{
						if (need_close)
						{
							close();
						}
						else
						{
							read_head();
						}
					}
				}	
				else
				{
					if (body_len + bytes_transferred>8192)
					{
						statas_code = 413;
						response_and_close(statas_code, self);
						return;
					}
					read_body(self, need_close, std::move(request), body_len);
				}					
			});
		}

		void read_body(const std::shared_ptr<connection>& self, bool need_close, request_t request, size_t body_len)
		{			
			//read http body
			boost::asio::async_read(socket_, read_buf_, boost::asio::transfer_exactly(body_len),
				[this, self, need_close, req = std::move(request)]
				(const boost::system::error_code& ec, std::size_t bytes_transferred)
			{
				if (ec)//on error
				{
					close();
					return;
				}

				int statas_code = 200;

				try
				{
					//callback to user
					//response
					//todo

					response_and_read(statas_code, need_close, self);
				}
				catch (const std::exception& ex)
				{
					std::cout << ex.what() << std::endl;
					statas_code = 400;
					response_and_close(statas_code, self);
				}
				catch (...)
				{
					statas_code = 400;
					response_and_close(statas_code, self);
				}
			});
		}

		bool need_close_conneciton(const request_t& request)
		{
			if (request.minor_version() == 0 && !request.has_keepalive_attr()) //short conneciton
			{
				return true;
			}

			if (request.minor_version() == 1 && request.has_close_attr())
			{
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
			if (!socket_.is_open())
				return;

			boost::system::error_code ignored_ec;
			socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			socket_.close(ignored_ec);
		}

		const std::string str_ = "HTTP/1.1 200 OK\r\n"
			"Date: Thu, 17 Nov 2016 08:38:08 GMT\r\n"
			"Server: Apache\r\n"
			"Content-Length: 11\r\n"
			"Connection: Keep-Alive\r\n"
			"Content-Type: text/html\r\n"
			"\r\n"
			"hello world";

		//异步发送消息，响应之后如果不继续read则该连接会被关闭
		void response_and_close(size_t status_code, const std::shared_ptr<connection>& self)
		{
			boost::asio::async_write(socket_, boost::asio::buffer(str_), [this, self](const boost::system::error_code& ec, std::size_t bytes_transferred)
			{
				if (ec != 0)
				{
					std::cout << ec.message() << std::endl;
				}
				close();
			});
		}

		void response_and_read(size_t status_code, bool need_close, const std::shared_ptr<connection>& self)
		{
			boost::asio::async_write(socket_, boost::asio::buffer(str_), [this, need_close, self](const boost::system::error_code& ec, std::size_t bytes_transferred)
			{
				if (ec != 0)
				{
					std::cout << ec.message() << std::endl;
				}

				if(need_close)
					close();
				else
					read_head();
			});
		}

	private:
		boost::asio::ip::tcp::socket socket_;
		boost::asio::streambuf read_buf_;
		const int MAX_LEN = 8192;
		const static std::map<unsigned int, std::string> http_status_table;
	};

	const std::map<unsigned int, std::string>
		connection::http_status_table =
	{
		{ 200, "200 OK" },
		{ 400, "Bad Request"},
		{ 404, "404 Not Found" },
		{ 413, "413 Request Entity Too Large" },
		{ 500, "500 Server Error" },
		{ 501, "501 Not Implemented" },
		{ 505, "505 HTTP Version Not Supported" }
	};
}

