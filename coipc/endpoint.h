#pragma once

#include "range.h"

#include <memory>
#include <stdexcept>
#include <string>

namespace coipc
{
	struct channel;

	typedef std::shared_ptr<channel> channel_ptr_t;


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


	struct channel
	{
		virtual void disconnect() throw() = 0;
		virtual void message(const_byte_range payload) = 0;
	};

	struct server
	{
		virtual channel_ptr_t create_session(channel &outbound) = 0;
	};



	channel_ptr_t connect_client(const std::string &typed_destination_endpoint_id, channel &inbound);
	std::shared_ptr<void> run_server(const std::string &typed_endpoint_id, const std::shared_ptr<server> &factory);


	inline initialization_failed::initialization_failed(const char *message)
		: std::runtime_error(message)
	{	}


	inline protocol_not_supported::protocol_not_supported(const char *message)
		: std::invalid_argument(message)
	{	}


	inline connection_refused::connection_refused(const char *message)
		: std::runtime_error(message)
	{	}
}
