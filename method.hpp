#pragma once
#include <string>
#include <array>
#include <unordered_map>

namespace timax
{
	namespace http
	{
		enum Method
		{
			GET, POST, PUT, DEL, OPTIONS, HEAD, TRACE, CONNECT, PATCH,
			INVALID = 0xffff
		}; //< enum Method

		namespace method 
		{

			/**
			* @brief Get the string representation from an HTTP
			* method code
			*
			* @param m:
			* The HTTP method code
			*
			* @return The string representation of the code
			*/
			static const std::string& str(const Method m)
			{

				const static std::array<std::string, 10> strings
				{
					{
						"GET", "POST", "PUT", "DELETE", "OPTIONS",
						"HEAD", "TRACE", "CONNECT", "PATCH", "INVALID"
					}
				};

				auto e = strings.size() - 1;

				if (((size_t)m >= 0) && ((size_t)m < e)) 
				{
					return strings[m];
				}

				return strings[e];
			}

			/**
			* @brief Get a code mapping from an HTTP
			* method string
			*
			* @param method:
			* The HTTP method code
			*
			* @return The code mapped to the method string
			**/
			inline Method code(const std::string& method) noexcept 
			{
				const static std::unordered_map<std::string, Method> code_map
				{
					{ "GET",     GET },
					{ "POST",    POST },
					{ "PUT",     PUT },
					{ "DELETE",  DEL },
					{ "OPTIONS", OPTIONS },
					{ "HEAD",    HEAD },
					{ "TRACE",   TRACE },
					{ "CONNECT", CONNECT },
					{ "PATCH",   PATCH }
				};

				auto it = code_map.find(method);

				return (it != code_map.end()) ? it->second : INVALID;
			}

			inline bool is_content_length_allowed(const Method method) noexcept 
			{
				return (method == POST) || (method == PUT);
			}

			inline bool is_content_length_required(const Method method) noexcept 
			{
				return (method == POST) || (method == PUT);
			}

		}
	}
}


