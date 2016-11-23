#include <iostream>
#include "connection.hpp"

using namespace timax;
void connection::read_head()
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
			response(statas_code, need_close, self, request);
			return;
		}

		size_t body_len = request.body_length();
		if (body_len == 0)
		{
			if (request.has_keepalive_attr())
			{
				response(statas_code, need_close, self, request);
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
				response(statas_code, need_close, self, request);
				return;
			}
			read_body(self, need_close, std::move(request), body_len);
		}
	});
}

void connection::read_body(const std::shared_ptr<connection>& self, bool need_close, request_t request, size_t body_len)
{
	//read http body
	boost::asio::async_read(socket_, read_buf_, boost::asio::transfer_exactly(body_len),
		[this, self, need_close, req = std::move(request)]
	(const boost::system::error_code& ec, std::size_t bytes_transferred) mutable
	{
		if (ec)//on error
		{
			close();
			return;
		}

		int statas_code = 200;

		try
		{
			response(statas_code, need_close, self, req);
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
			statas_code = 400;
			response(statas_code, need_close, self, req);
		}
		catch (...)
		{
			statas_code = 400;
			response(statas_code, need_close, self, req);
		}
	});
}

void connection::response(size_t status_code, bool need_close, const std::shared_ptr<connection>& self, request_t& request)
{
	auto response = std::make_shared<response_t>(this, request.minor_version());
	response->set_status(status_code);
	//callback to user
	bool success = server_->process_route(&request, response.get());
	if(!success)
		need_close = true;

	response->send_response(need_close);
}

void connection::write(const std::shared_ptr<response_t>& response, const std::vector<boost::asio::const_buffer>& buffers, bool need_close)
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