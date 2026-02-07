#pragma once

#include "endpoint.h"

namespace coipc
{
	namespace com
	{
		channel_ptr_t connect_client(const char *destination_endpoint_id, channel &inbound);
		std::shared_ptr<void> run_server(const char *endpoint_id, const std::shared_ptr<server> &factory);
	}
}
