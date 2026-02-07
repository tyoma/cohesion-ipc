#pragma once

#include "endpoint.h"

#include <vector>

namespace coipc
{
	namespace spawn
	{
		struct server_exe_not_found : connection_refused
		{
			server_exe_not_found(const char *message);
		};

		channel_ptr_t connect_client(const std::string &spawned_path, const std::vector<std::string> &arguments,
			channel &inbound);
			
		channel_ptr_t create_session(const std::vector<std::string> &arguments, channel &outbound);
	}
}
