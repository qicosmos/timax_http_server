#include "response.hpp"

using namespace timax;
const std::unordered_map<unsigned int, std::string> response_t::HTTP_STATUS_TABLE =
{
	{ 200, "200 OK" },
	{ 400, "Bad Request" },
	{ 404, "404 Not Found" },
	{ 413, "413 Request Entity Too Large" },
	{ 500, "500 Server Error" },
	{ 501, "501 Not Implemented" },
	{ 505, "505 HTTP Version Not Supported" }
};

//std::string g_str = "HTTP/1.0 200 OK\r\n"
//"Content-Length: 4\r\n"
//"Content-Type: text/html\r\n"
//"Connection: Keep-Alive\r\n\r\n"
//"TEST";

std::string g_str = 
"HTTP/1.0 200 OK\r\n"
"Content-Length: 4\r\n"
"Server: timax_server/0.1\r\n"
"Date: Mon, 28 Nov 2016 06:20:08 GMT\r\n"
"\r\n"
"test";

void response_t::send_response(bool need_close)
{
	assert(conn_);

	//std::string protocal = "";
	//if (minor_version_ == 1)
	//	protocal = "HTTP/1.1 ";
	//else
	//	protocal = "HTTP/1.0 ";

	//status_line_ = protocal + HTTP_STATUS_TABLE.at(status_code_) + "\r\n";
	//std::size_t size = boost::asio::buffer_size(resource_buffer_);
	//header_str_ = "Content-Length: " + boost::lexical_cast<std::string>(size) + "\r\n";

	//if (!need_close)
	//{
	//	header_.emplace("Connection", "keep-alive");
	//}

	//for (auto& iter : header_)
	//{
	//	header_str_ += iter.first;
	//	header_str_ += ": ";
	//	header_str_ += iter.second;
	//	header_str_ += "\r\n";
	//}

	//header_str_ += "\r\n";

	//response_buffers_.push_back(boost::asio::buffer(status_line_));
	//response_buffers_.push_back(boost::asio::buffer(header_str_));
	//if (size>0)
	//	response_buffers_.push_back(resource_buffer_);
	response_buffers_.push_back(boost::asio::buffer(g_str));
	auto self = this->shared_from_this();
	conn_->write(self, response_buffers_, need_close);
}