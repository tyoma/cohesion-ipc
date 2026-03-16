#pragma once

#include "endpoint.h"

#include <vector>

namespace coipc
{
	namespace spawn
	{
		channel_ptr_t connect_client(const std::string &spawned_path, const std::vector<std::string> &arguments,
			channel &inbound);
			
		channel_ptr_t create_session(const std::vector<std::string> &arguments, channel &outbound);
	}
}
