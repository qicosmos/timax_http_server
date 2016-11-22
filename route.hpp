#pragma once
#include "request.hpp"
#include "response.hpp"
#include "path_to_regex.hpp"

namespace timax
{
	typedef void(*Handler)(request_t*, response_t*);
	using route_expr = std::regex;

	struct route 
	{
		route(const std::string& ex, Handler e) : path{ ex }, job{ e }
		{
			expr = path2regex::path_to_regex(path, keys);
		}

		std::string path;
		route_expr  expr;
		Handler   job;
		path2regex::Keys keys;
		size_t      hits{ 0U };
	};

	inline bool operator < (const route& lhs, const route& rhs) noexcept
	{
		return rhs.hits < lhs.hits;
	}
}


