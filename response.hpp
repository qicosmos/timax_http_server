#pragma once
#include <string>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include "connection.hpp"

namespace timax
{
	class connection;
	class response_t : public std::enable_shared_from_this<response_t>
	{
	public:
		response_t(connection* conn, int minor_version = 1) : conn_(conn), minor_version_(minor_version)
		{
		}

		void add_header(const std::string& key, const std::string& val)
		{
			header_.emplace(key, val);
		}

		void add_body(const char* data, size_t size)
		{
			buffer_.sputn(data, size); 
			resource_buffer_ = { boost::asio::buffer_cast<const char*>(buffer_.data()), buffer_.size() };
		}

		void add_body(const std::string& body)
		{
			add_body(body.data(), body.size());
		}

		void set_status(unsigned int status_code, bool need_close = false)
		{
			status_code_ = status_code;
			need_close_ = need_close;
		}

	private:
		friend class connection;
		void send_response(bool need_close = true);
		void add_resource(const char* data, size_t size)
		{
			resource_buffer_ = { data, size };
		}

		unsigned int status_code_;
		bool need_close_;
		int minor_version_;
		std::string status_line_;
		std::string header_str_;		
		boost::asio::streambuf buffer_;
		boost::asio::const_buffer resource_buffer_;
		std::unordered_map<std::string, std::string> header_;
		std::vector<boost::asio::const_buffer> response_buffers_;
		connection* conn_;
		const static std::unordered_map<unsigned int, std::string> HTTP_STATUS_TABLE;
	};
}


