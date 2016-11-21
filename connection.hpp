#pragma once
#include <boost/asio.hpp>
#include "request.hpp"
#include "response.hpp"

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
			read_buf_.consume(read_buf_.size());
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

					close();
					return;
				}

				request_t request;
				int r = request.parse(boost::asio::buffer_cast<const char*>(read_buf_.data()), bytes_transferred);
				bool need_close = need_close_conneciton(request);

				if (r < 0)
				{
					statas_code = 400;
					response(statas_code, need_close, self, request.minor_version());
					return;
				}

				size_t body_len = request.body_length();
				if (body_len == 0)
				{
					if (request.has_keepalive_attr())
					{
						response(statas_code, need_close, self, request.minor_version());
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
						response(statas_code, need_close, self, request.minor_version());
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
					response(statas_code, need_close, self, req.minor_version());
				}
				catch (const std::exception& ex)
				{
					std::cout << ex.what() << std::endl;
					statas_code = 400;
					response(statas_code, need_close, self, req.minor_version());
				}
				catch (...)
				{
					statas_code = 400;
					response(statas_code, need_close, self, req.minor_version());
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

		void response(size_t status_code, bool need_close, const std::shared_ptr<connection>& self, int minor_version = 1)
		{
			auto response = std::make_shared<response_t>();
			//callback to user
			std::string body = "hello";
			response->add_body(body.data(), body.size());
			response->init_header(status_code, minor_version);

			response->set_callback([this, &response](auto buffers, auto need_close)
			{
				write(response, std::move(buffers), need_close);
			});

			response->send_response(need_close);
		}

		void write(const std::shared_ptr<response_t>& response,const std::vector<boost::asio::const_buffer>& buffers, bool need_close = true)
		{
			auto self = this->shared_from_this();
			boost::asio::async_write(socket_, buffers,
				[this, self, response, need_close](const boost::system::error_code& ec, std::size_t bytes_transferred)
			{
				if (ec != 0)
				{
					std::cout << ec.message() << std::endl;
				}

				if (need_close)
					close();
				else
					read_head();
			});
		}

	private:
		boost::asio::ip::tcp::socket socket_;
		boost::asio::streambuf read_buf_;
		const int MAX_LEN = 8192;
	};
}

