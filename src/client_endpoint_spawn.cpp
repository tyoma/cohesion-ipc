#include <coipc/spawn/endpoint.h>

using namespace std;

namespace coipc
{
	namespace spawn
	{
		server_exe_not_found::server_exe_not_found(const char *message)
			: connection_refused(message)
		{	}


		client_session::client_session(const string &spawned_path, const vector<string> &arguments, channel &inbound)
		{
			auto pipes = spawn(spawned_path, arguments);
			auto inbound_stream = pipes.second;

			_outbound = pipes.first;
			_thread.reset(new mt::thread([this, &inbound, inbound_stream] {
				unsigned int n;
				vector<uint8_t> buffer;

				while (fread(&n, sizeof n, 1, inbound_stream.get()))
				{
					buffer.resize(n);
					fread(buffer.data(), 1, n, inbound_stream.get());
					inbound.message(const_byte_range(buffer.data(), buffer.size()));
				}
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


		channel_ptr_t connect_client(const string &spawned_path, const vector<string> &arguments, channel &inbound)
		{	return make_shared<client_session>(spawned_path, arguments, inbound);	}
	}
}
