#include <coipc/endpoint_com.h>
#include <coipc/endpoint_sockets.h>

#include "helpers.h"

#include <functional>

using namespace std;

namespace coipc
{
	shared_ptr<void> run_server(const string &typed_endpoint_id, const shared_ptr<server> &factory)
	{
		typedef function<shared_ptr<void> (const char *typed_endpoint_id, const shared_ptr<server> &factory)>
			server_endpoint_factory_t;

		constructor<server_endpoint_factory_t> c_constructors[] = {
#ifdef _WIN32
			{ "com", &com::run_server },
#endif
			{ "sockets", &sockets::run_server },
		};

		string endpoint_id;
		server_endpoint_factory_t ctor = select(c_constructors, typed_endpoint_id, endpoint_id);

		return ctor(endpoint_id.c_str(), factory);
	}
}
