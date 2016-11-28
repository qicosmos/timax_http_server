#pragma once
#include <memory>
#include <boost/asio.hpp>
#include "io_service_pool.hpp"
#include "connection.hpp"
#include "http_router.hpp"
using namespace boost::asio::ip;

namespace timax
{
	class connection_t;
	class response_t;

	class server_t
	{
	public:

		server_t(uint16_t port, size_t pool_size = std::thread::hardware_concurrency()) : ios_pool_(pool_size),
			acceptor_(ios_pool_.get_io_service(), tcp::endpoint{ tcp::v4(), port })
		{
		}

		~server_t()
		{
			stop();
		}

		void start()
		{
			do_accept();
			ios_pool_.start();
		}

		void stop()
		{
			ios_pool_.stop();
		}

		http_router& router() noexcept 
		{
			return router_;
		}

		server_t& set_router(const http_router& router)
		{
			router_ = router;
			return *this;
		}

		bool process_route(request_t* req, response_t* res);

		server_t& set_static_dir(const std::string& dir)
		{
			static_dir_ = dir;
			return *this;
		}

		const std::string& static_dir() const
		{
			return static_dir_;
		}

	private:
		void do_accept();

	private:
		io_service_pool				ios_pool_;
		tcp::acceptor				acceptor_;
		http_router					router_;
		std::shared_ptr<connection> conn_;
		std::string static_dir_;
	};
}


