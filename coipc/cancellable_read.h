#pragma once

#include <cstdio>
#include <memory>

namespace coipc
{
	struct cancelled_exception : public std::exception
	{
    };

	class cancellable_read
	{
	public:
		struct platform;

	public:
		cancellable_read(platform &implementation = default_implementation);
		~cancellable_read();
		
		std::size_t read(std::FILE &file, void *buffer, std::size_t size);
		
		// Post-condition: all read requests made past this call will throw cancelled_exception.
		// Note: if there's an ongoing or a starting read() operation - it'll exit with cancelled_exception.
		void cancel();

	public:
        static platform &default_implementation;
		
	private:
		class impl;
		
	private:
		std::unique_ptr<impl> _impl;
	};

	struct cancellable_read::platform
	{
		virtual std::size_t read(std::FILE &file, void *buffer, std::size_t size) = 0;
		virtual bool cancel_io(void *thread_handle) = 0;
    };
}
