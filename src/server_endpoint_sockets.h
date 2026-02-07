#pragma once

#include "socket_helpers.h"

#include <coipc/endpoint_sockets.h>
#include <coipc/types.h>

#include <mt/thread.h>
#include <list>
#include <vector>

namespace coipc
{
	namespace sockets
	{
		class socket_handler : public /*outbound*/ channel, noncopyable
		{
		public:
			enum status { proceed, remove_this, exit, };
			typedef std::function<status (socket_handler &self, const socket_handle &s)> handler_t;
			typedef std::shared_ptr<socket_handler> ptr_t;

		public:
			socket_handler(unsigned id_, socket_handle &s, const socket_handle &aux_socket,
				const handler_t &initial_handler);
			~socket_handler() throw();

			template <typename ContainerT>
			static void run(ContainerT &handlers);

			virtual void disconnect() throw();
			virtual void message(const_byte_range payload);

		public:
			const unsigned id;
			handler_t handler;

		private:
			socket_handle _socket;
			const socket_handle &_aux_socket;
		};

		class server
		{
		public:
			server(const char *endpoint_id, const std::shared_ptr<coipc::server> &factory);
			~server();

		private:
			enum {
				max_backlog = 5,
				init_magic = 0xFFFFFFFE,
			};

			typedef std::list<socket_handler::ptr_t> handlers_t;

		private:
			void worker();
			socket_handler::status handle_preinit(socket_handler &h, const socket_handle &s);
			socket_handler::status accept_preinit(const socket_handle &s);
			socket_handler::status accept_regular(const socket_handle &s);
			socket_handler::status handle_session(const socket_handle &s, const channel_ptr_t &inbound);
			socket_handler::status handle_aux(const socket_handle &s);

			static int connect_aux(unsigned short port);

		private:
			sockets_initializer _initializer;
			std::shared_ptr<coipc::server> _factory;
			socket_handle _aux_socket;
			unsigned _next_id;
			handlers_t _handlers;
			std::unique_ptr<mt::thread> _server_thread;
			std::vector<byte> _buffer;
		};
	}
}
