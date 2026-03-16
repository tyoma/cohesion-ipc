#pragma once

#include "endpoint.h"

#include <functional>

namespace coipc
{
	struct stream
	{
		typedef std::function<channel_ptr_t (channel &outbound)> session_factory_t;

		static std::shared_ptr<void> connect(FILE &inbound, FILE &outbound, const session_factory_t &new_server_session);
	};
}
