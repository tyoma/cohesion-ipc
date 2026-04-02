#pragma once

#include "endpoint.h"

#include <functional>
#include <vector>

namespace coipc
{
	namespace spawn
	{
		typedef std::function<void (int exit_code)> exit_handler_t;

		channel_ptr_t connect_client(const std::string &spawned_path, const std::vector<std::string> &arguments,
			const std::vector<std::string> &extra_environment, channel &inbound, exit_handler_t &&exit_handler);

		channel_ptr_t create_session(const std::vector<std::string> &arguments, channel &outbound);
	}
}
