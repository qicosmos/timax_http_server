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
		using Callback = std::function<void(const std::vector<boost::asio::const_buffer>&, bool)>;

		response_t(connection* conn) : conn_(conn)
		{
		}

		~response_t()
		{
		}

		void init_header(unsigned int status_code, int minor_version);

		void add_header(const std::string& key, const std::string& val)
		{
			header_.emplace(key, val);
		}

		void add_body(const char* data, size_t size)
		{
			buffer_.sputn(data, size); 
		}

		void send_response(bool need_close = true);

		void set_callback(Callback callback)
		{
			callback_ = callback;
		}

		const std::vector<boost::asio::const_buffer> get_buffers() const
		{
			return response_buffers_;
		}

	private:
		std::string status_line_;
		std::string header_str_;		
		boost::asio::streambuf buffer_;
		std::unordered_map<std::string, std::string> header_;
		std::vector<boost::asio::const_buffer> response_buffers_;
		Callback callback_;
		connection* conn_;
		const static std::unordered_map<unsigned int, std::string> HTTP_STATUS_TABLE;
	};
}


