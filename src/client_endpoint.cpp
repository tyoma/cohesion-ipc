#include <coipc/endpoint_com.h>
#include <coipc/endpoint_sockets.h>

#include "helpers.h"

#include <functional>

using namespace std;

namespace coipc
{
	channel_ptr_t connect_client(const string &typed_destination_endpoint_id, channel &inbound)
	{
		typedef function<channel_ptr_t (const char *destination_endpoint_id, channel &inbound)>
			client_endpoint_factory_t;

		constructor<client_endpoint_factory_t> c_constructors[] = {
#ifdef _WIN32
			{ "com", &com::connect_client },
#endif
			{ "sockets", &sockets::connect_client },
		};

		string endpoint_id;
		client_endpoint_factory_t ctor = select(c_constructors, typed_destination_endpoint_id, endpoint_id);

		return ctor(endpoint_id.c_str(), inbound);
	}
}
