#pragma once
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>
#include <thread>
#include <list>

namespace timax 
{
	class io_service_pool : boost::noncopyable
	{
	public:
		using ios_work_ptr = std::unique_ptr<boost::asio::io_service::work>;
	
		class ios_worker
		{
		public:
			ios_worker()
				: ios_()
				, work_(std::make_unique<boost::asio::io_service::work>(ios_))
			{}

			void start()
			{
				worker_ = std::move(std::thread{ boost::bind(&boost::asio::io_service::run, &ios_) });
			}

			void stop()
			{
				work_.reset();
				if (!ios_.stopped())
					ios_.stop();
			}

			void wait()
			{
				if (worker_.joinable())
					worker_.join();
			}

			auto& get_io_service()
			{
				return ios_;
			}

		private:
			boost::asio::io_service ios_;
			ios_work_ptr	work_;
			std::thread	worker_;
		};

		using iterator = std::list<ios_worker>::iterator;

	public:
		explicit io_service_pool(size_t pool_size)
			: ios_workers_(pool_size)
			, next_io_service_(ios_workers_.begin())
		{
		}

		~io_service_pool()
		{
			stop();
		}

		void start()
		{
			for (auto& ios_worker : ios_workers_)
				ios_worker.start();
		}

		void stop()
		{
			for (auto& ios : ios_workers_)
				ios.stop();

			for (auto& ios : ios_workers_)
				ios.wait();
		}

		auto& get_io_service()
		{
			auto current = next_io_service_++;
			if (ios_workers_.end() == next_io_service_)
			{
				next_io_service_ = ios_workers_.begin();
			}

			return current->get_io_service();
		}
		
	private:
		std::list<ios_worker>		ios_workers_;
		iterator					next_io_service_;
	};
}