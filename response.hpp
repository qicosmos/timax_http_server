#pragma once
#include <string>
#include <unordered_map>
#include <boost/lexical_cast.hpp>

namespace timax
{
	class response_t
	{
	public:
		using Callback = std::function<void(const std::vector<boost::asio::const_buffer>&, bool)>;

		response_t()
		{
		}

		~response_t()
		{
		}

		void init_header(unsigned int status_code, int minor_version)
		{
			header_str_.clear();
			std::string protocal = "";
			if (minor_version == 1)
				protocal = "HTTP/1.1 ";
			else
				protocal = "HTTP/1.0 ";

			status_line_ = protocal + HTTP_STATUS_TABLE.at(status_code) + "\r\n";

			header_str_ = "Content-Length: " + boost::lexical_cast<std::string>(buffer_.size()) + "\r\n";

			for (auto& iter : header_)
			{
				header_str_ += iter.first;
				header_str_ += ": ";
				header_str_ += iter.second;
				header_str_ += "\r\n";
			}

			header_str_ += "\r\n";
		}

		void add_header(const std::string& key, const std::string& val)
		{
			header_.emplace(key, val);
		}

		void add_body(const char* data, size_t size)
		{
			buffer_.sputn(data, size); 
		}

		void send_response(bool need_close = true)
		{
			assert(callback_);

			response_buffers_.push_back(boost::asio::buffer(status_line_));
			response_buffers_.push_back(boost::asio::buffer(header_str_));
			if(buffer_.size()>0)
				response_buffers_.push_back(boost::asio::buffer(buffer_.data(), buffer_.size()));

			callback_(response_buffers_, need_close);
		}

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
		const static std::unordered_map<unsigned int, std::string> HTTP_STATUS_TABLE;
	};

	const std::unordered_map<unsigned int, std::string>
		response_t::HTTP_STATUS_TABLE =
	{
		{ 200, "200 OK" },
		{ 400, "Bad Request" },
		{ 404, "404 Not Found" },
		{ 413, "413 Request Entity Too Large" },
		{ 500, "500 Server Error" },
		{ 501, "501 Not Implemented" },
		{ 505, "505 HTTP Version Not Supported" }
	};
}


