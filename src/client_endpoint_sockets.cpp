#include <coipc/endpoint_sockets.h>

#include "socket_helpers.h"

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
			void worker(channel *inbound);
			static int open(const host_port &hp);

		private:
			sockets_initializer _initializer;
			socket_handle _socket;
			unique_ptr<mt::thread> _thread;
		};



		client_session::client_session(const host_port &hp, channel &inbound)
			: _socket(open(hp))
		{	_thread.reset(new mt::thread(bind(&client_session::worker, this, &inbound)));	}

		client_session::~client_session()
		{
			::shutdown(_socket, SHUT_RDWR);
			_socket.reset();
			_thread->join();
		}

		void client_session::disconnect() throw()
		{	}

		void client_session::message(const_byte_range payload)
		{
			unsigned int size_ = static_cast<unsigned int>(payload.length());
			sockets::byte_representation<unsigned int> size;

			size.value = size_;
			size.reorder();
			::send(_socket, size.bytes, sizeof(size.bytes), MSG_NOSIGNAL);
			::send(_socket, reinterpret_cast<const char *>(payload.begin()), size_, MSG_NOSIGNAL);
		}

		void client_session::worker(channel *inbound)
		{
			byte_representation<unsigned int> size;
			vector<uint8_t> buffer;

			LOG(PREAMBLE "processing thread started...") % A(inbound);
			while (::recv(_socket, size.bytes, sizeof(size), MSG_WAITALL) == (int)sizeof(size))
			{
				size.reorder();
				buffer.resize(size.value);
				::recv(_socket, (char *)&buffer[0], size.value, MSG_WAITALL);
				inbound->message(const_byte_range(&buffer[0], buffer.size()));
			}
			LOG(PREAMBLE "disconnecting from the server...") % A(inbound);
			inbound->disconnect();
			LOG(PREAMBLE "processing thread ended.");
		}

		int client_session::open(const host_port &hp)
		{
			sockaddr_in service = make_sockaddr_in(hp.host.c_str(), hp.port);
			int hsocket = static_cast<int>(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

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

			return channel_ptr_t(new client_session(hp, inbound));
		}
	}
}
