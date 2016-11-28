#pragma once
#include <boost/utility/string_ref.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <unordered_map>
#include "picohttpparser.h"

namespace timax
{
	class request_t
	{
	public:
		boost::string_ref method() const
		{
			return method_;
		}

		boost::string_ref url() const
		{
			return url_;
		}

		int minor_version() const
		{
			return minor_version_;
		}

		const std::map<boost::string_ref, boost::string_ref>& headers() const
		{
			return headers_;
		}

		const std::unordered_map<std::string, std::string>& params() const
		{
			return params_;
		}

		void set_params(std::unordered_map<std::string, std::string> params)
		{
			params_ = std::move(params);
		}

		int parse(const std::string& str, int last_len)
		{
			return parse(str.c_str(), str.size(), last_len);
		}

		int parse(const char* buf, size_t size, int last_len)
		{
			const char *method;
			size_t method_len;
			const char *path;
			size_t path_len;
			int minor_version;
			struct phr_header headers[15];
			size_t num_headers = 15;

			//int last_len = 0;
			auto r = phr_parse_request(buf, size, &method, &method_len, &path, &path_len, &minor_version, headers, &num_headers, last_len);
			if (r < 0)
				return r;

			for (size_t i = 0; i < num_headers; i++)
			{
				phr_header head = headers[i];
				boost::string_ref key = { head.name, head.name_len };
				boost::string_ref val = { head.value, head.value_len };
				headers_.emplace(key, val);
			}

			auto it = headers_.find("Content-Length");
			if (it != headers_.end())
			{
				try
				{
					body_len_ = boost::lexical_cast<unsigned int>(it->second);
				}
				catch (const std::exception&)
				{
					r = -1;
				}
			}

			if (r < 0)
				return r;

			method_ = { method, method_len };
			url_ = { path, path_len };
			minor_version_ = minor_version;

			return r; //- 2: not complete,  - 1: parse error 
		}

		size_t body_length() const
		{
			return body_len_;
		}

		bool has_keepalive_attr() const
		{
			auto it = headers_.find("Connection");
			if (it == headers_.end())
				return false;

			if (it->second.size() != 10)
				return false;

			const char* data = "keep-alive";
			for (size_t i = 0; i < 10; i++)
			{
				if (data[i] != std::tolower(it->second[i]))
					return false;
			}

			return true;
		}

		bool has_close_attr() const
		{
			auto it = headers_.find("Connection");
			if (it == headers_.end())
				return false;

			if (it->second.size() != 5)
				return false;

			const char* data = "close";
			for (size_t i = 0; i < 5; i++)
			{
				if (data[i] != std::tolower(it->second[i]))
					return false;
			}

			return true;
		}

	private:
		friend class connection;
		friend class server_t;
		boost::string_ref method_;
		boost::string_ref url_;
		int minor_version_ = 0;
		size_t body_len_ = 0;
		std::map<boost::string_ref, boost::string_ref> headers_;

		std::unordered_map<std::string, std::string> params_;
	};
}

