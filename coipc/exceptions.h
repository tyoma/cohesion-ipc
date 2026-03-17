#pragma once

#include <stdexcept>

namespace coipc
{
	struct cancelled_exception : std::exception
	{
		cancelled_exception();
	};

	struct initialization_failed : std::runtime_error
	{
		initialization_failed(const char *message);
	};

	struct protocol_not_supported : std::invalid_argument
	{
		protocol_not_supported(const char *message);
	};

	struct connection_refused : std::runtime_error
	{
		connection_refused(const char *message);
	};

	struct server_exe_not_found : connection_refused
	{
		server_exe_not_found(const char *message);
	};

	struct insufficient_buffer_error : std::runtime_error
	{
		insufficient_buffer_error(std::size_t requested_, std::size_t available_);

		std::size_t requested, available;
	};



	inline cancelled_exception::cancelled_exception()
	{	}


	inline initialization_failed::initialization_failed(const char *message)
		: std::runtime_error(message)
	{	}


	inline protocol_not_supported::protocol_not_supported(const char *message)
		: std::invalid_argument(message)
	{	}


	inline connection_refused::connection_refused(const char *message)
		: std::runtime_error(message)
	{	}


	inline server_exe_not_found::server_exe_not_found(const char *message)
		: connection_refused(message)
	{	}

	inline insufficient_buffer_error::insufficient_buffer_error(std::size_t requested_, std::size_t available_)
		: std::runtime_error("buffer is smaller than requested to read"), requested(requested_), available(available_)
	{	}
}
