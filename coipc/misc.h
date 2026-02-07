#pragma once

#include "types.h"

#include <string>

namespace coipc
{
	struct ip_v4
	{
		unsigned char components[4];
	};

	const ip_v4 localhost = { { 127, 0, 0, 1 } };
	const ip_v4 all_interfaces = { { 0, 0, 0, 0 } };

	std::string sockets_endpoint_id(ip_v4 host, unsigned short port);
	std::string com_endpoint_id(const guid_t &id);

	std::string to_string(const guid_t &id);
	guid_t guid_from_string(const char *str);
}
