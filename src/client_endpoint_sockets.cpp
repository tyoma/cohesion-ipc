#include <coipc/endpoint_sockets.h>

#include "helpers.h"
#include "socket_helpers.h"

#include <coipc/exceptions.h>

#include <arpa/inet.h>
#include <logger/log.h>
#include <mt/thread.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <vector>

#define PREAMBLE "IPC socket client: "

using namespace std;

namespace coipc
{
	namespace sockets
	{
		class client_session : public channel
		{
		public:
			client_session(const host_port &hp, channel &inbound);
			~client_session();

			virtual void disconnect() throw();
			virtual void message(const_byte_range payload);

		private:
			static int open(const host_port &hp);

		private:
			sockets_initializer _initializer;
			socket_handle _socket;
			mt::thread _thread;
		};



		client_session::client_session(const host_port &hp, channel &inbound)
			: _socket(open(hp)), _thread([this, &inbound] {
				LOG(PREAMBLE "processing thread started...") % A(&inbound);
				read_messages(inbound, [&] (void *buffer, unsigned int size) {
					return ::recv(_socket, static_cast<char *>(buffer), size, MSG_WAITALL);
				}, true);
				LOG(PREAMBLE "disconnecting from the server...") % A(&inbound);
				inbound.disconnect();
				LOG(PREAMBLE "processing thread ended.");
			})
		{	}

		client_session::~client_session()
		{
			::shutdown(_socket, SHUT_RDWR);
			_socket.reset();
			_thread.join();
		}

		void client_session::disconnect() throw()
		{	}

		void client_session::message(const_byte_range payload)
		{
			const auto size_ = static_cast<unsigned int>(payload.length());
			byte_representation<unsigned int> size;

			size.value = size_;
			size.reorder();
			::send(_socket, size.bytes, sizeof(size.bytes), MSG_NOSIGNAL);
			::send(_socket, reinterpret_cast<const char *>(payload.data()), size_, MSG_NOSIGNAL);
		}

		int client_session::open(const host_port &hp)
		{
			auto service = make_sockaddr_in(hp.host.c_str(), hp.port);
			auto hsocket = static_cast<int>(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

			if (-1 == hsocket)
				throw initialization_failed("socket creation failed");
			if (::connect(hsocket, (sockaddr *)&service, sizeof(service)))
			{
				::close(hsocket);
				throw connection_refused(hp.host.c_str());
			}
			return hsocket;
		}


		channel_ptr_t connect_client(const char *destination_endpoint_id, channel &inbound)
		{
			host_port hp(destination_endpoint_id);

			return make_shared<client_session>(hp, inbound);
		}
	}
}
