#include <coipc/spawn/endpoint.h>

#include "helpers.h"

#include <coipc/endpoint_spawn.h>
#include <coipc/exceptions.h>

using namespace std;

namespace coipc
{
	namespace spawn
	{
		client_session::client_session(const string &spawned_path, const vector<string> &arguments,
			const vector<string> &extra_environment, channel &inbound, exit_handler_t &&exit_handler)
		{
			auto spawned = spawn(spawned_path, arguments, extra_environment, std::move(exit_handler));

			_outbound = std::move(spawned.to);
			_thread.reset(new mt::thread([this, &inbound, spawned] {
				read_messages(inbound, [&] (void *buffer, size_t size) {
					return fread(buffer, 1, size, spawned.from.get());
				}, false);
				if (_outbound) // We didn't close the pipe - the server disconnected by itself.
					inbound.disconnect();
			}));
		}

		client_session::~client_session()
		{
			_outbound.reset(); // This breaks the server's listen loop, exits it, which causes its stdout to get closed.
			_thread->join();
		}

		void client_session::disconnect() throw()
		{	}

		void client_session::message(const_byte_range payload)
		{
			const auto size = static_cast<unsigned int>(payload.length());

			fwrite(&size, sizeof size, 1, _outbound.get());
			fwrite(payload.begin(), 1, payload.length(), _outbound.get());
			fflush(_outbound.get());
		}


		channel_ptr_t connect_client(const string &spawned_path, const vector<string> &arguments,
			const vector<string> &extra_environment, channel &inbound, exit_handler_t &&exit_handler)
		{
			return make_shared<client_session>(spawned_path, arguments, extra_environment, inbound,
				std::move(exit_handler));
		}
	}
}
