#include <coipc/misc.h>

#include <cstdio>

using namespace std;

namespace coipc
{
	string sockets_endpoint_id(ip_v4 host, unsigned short port)
	{
		char endpoint_id[32] = { 0 };

		snprintf(endpoint_id, sizeof endpoint_id, "sockets|%d.%d.%d.%d:%d",
			host.components[0], host.components[1], host.components[2], host.components[3], port);
		return endpoint_id;
	}

	string com_endpoint_id(const guid_t &id)
	{	return "com|" + to_string(id);	}

	string to_string(const guid_t &id)
	{
		const auto &v = id.values;
		char str[64] = { 0 };

		snprintf(str, sizeof str, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			*reinterpret_cast<const unsigned int *>(&v[0]),
			*reinterpret_cast<const unsigned short *>(&v[4]),
			*reinterpret_cast<const unsigned short *>(&v[6]),
			v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]);
		return str;
	}

	guid_t guid_from_string(const char *str)
	{
		struct {
			int padding1;
			guid_t id;
			int padding2;
		} x;
		byte (&v)[16] = x.id.values;

		sscanf(str, "%08X-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
			reinterpret_cast<unsigned int *>(&v[0]),
			reinterpret_cast<unsigned short *>(&v[4]),  reinterpret_cast<unsigned short *>(&v[6]),
			&v[8], &v[9], &v[10], &v[11], &v[12], &v[13], &v[14], &v[15]);
		return x.id;
	}
}
