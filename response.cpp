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

void response_t::send_response(bool need_close)
{
	assert(conn_);

	std::string protocal = "";
	if (minor_version_ == 1)
		protocal = "HTTP/1.1 ";
	else
		protocal = "HTTP/1.0 ";

	status_line_ = protocal + HTTP_STATUS_TABLE.at(status_code_) + "\r\n";

	header_str_ = "Content-Length: " + boost::lexical_cast<std::string>(buffer_.size()) + "\r\n";

	for (auto& iter : header_)
	{
		header_str_ += iter.first;
		header_str_ += ": ";
		header_str_ += iter.second;
		header_str_ += "\r\n";
	}

	header_str_ += "\r\n";

	response_buffers_.push_back(boost::asio::buffer(status_line_));
	response_buffers_.push_back(boost::asio::buffer(header_str_));
	if (buffer_.size()>0)
		response_buffers_.push_back(boost::asio::buffer(buffer_.data(), buffer_.size()));

	auto self = this->shared_from_this();
	conn_->write(self, response_buffers_, need_close);
}