#pragma once
#include <unordered_map>
#include <regex>
#include "method.hpp"
#include "route.hpp"

namespace timax
{
	#define DEFINE_HANDLER(method, http_method)\
	http_router& on_##method(const std::string& route, Handler handler)\
	{\
		route_table_[##http_method].emplace_back(route, handler);\
		return *this;\
	}

	class http_router
	{
	public:

		http_router()
		{
		}

		~http_router()
		{
		}

		struct parsed_route_t
		{
			Handler job;
			std::map<std::string, std::string> parsed_values;
		};

		inline parsed_route_t match(http::Method method, const std::string& path)
		{
			auto routes = route_table_[method];

			if (routes.empty()) {
				throw std::runtime_error("No routes for method " + http::method::str(method));
			}

			for (auto& route : routes) 
			{
				if (std::regex_match(path, route.expr)) 
				{
					++route.hits;

					//Set the pairs in params:
					std::map<std::string, std::string> params;
					std::smatch res;

					for (std::sregex_iterator i = std::sregex_iterator{ path.begin(), path.end(), route.expr };
					i != std::sregex_iterator{}; ++i) {
						res = *i;
					}

					// First parameter/value is in res[1], second in res[2], and so on
					for (size_t i = 0; i < route.keys.size(); i++)
						params.emplace(route.keys[i].name, res[i + 1]);

					parsed_route_t parsed_route;
					parsed_route.job = route.job;
					parsed_route.parsed_values = std::move(params);

					return parsed_route;
				}
			}

			throw std::runtime_error("No matching route for " + http::method::str(method) + " " + path);
		}

		http_router& on_get(const std::string& route, Handler handler)
		{
			route_table_[http::GET].emplace_back(route, handler);
			return *this;
		}

		DEFINE_HANDLER(post, http::POST);
		DEFINE_HANDLER(head, http::HEAD);
		DEFINE_HANDLER(put, http::PUT);
		DEFINE_HANDLER(connect, http::CONNECT);
		DEFINE_HANDLER(delete, http::DEL);
		DEFINE_HANDLER(invalid, http::INVALID);
		DEFINE_HANDLER(options, http::OPTIONS);
		DEFINE_HANDLER(patch, http::PATCH);
		DEFINE_HANDLER(trace, http::TRACE);

	private:
		std::unordered_map<http::Method, std::vector<route>> route_table_;
	};
}


