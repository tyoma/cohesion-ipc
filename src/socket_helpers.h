#pragma once

#include <coipc/noncopyable.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace coipc
{
	namespace sockets
	{
		const char c_localhost[] = "127.0.0.1";

		struct sockets_initializer : noncopyable
		{
			sockets_initializer();
			~sockets_initializer();
		};

		struct host_port
		{
			host_port(const char *address, const char *default_host = c_localhost);

			std::string host;
			unsigned short port;
		};

		class socket_handle : noncopyable
		{
		public:
			template <typename T>
			explicit socket_handle(T s);
			socket_handle(socket_handle &other);
			~socket_handle();

			void reset() throw();
			void reset(int s);
			operator int() const;

		private:
			int _socket;
		};



		inline host_port::host_port(const char *address, const char *default_host)
		{
			if (const char *delim = strchr(address, ':'))
				host.assign(address, delim), port = static_cast<unsigned short>(atoi(delim + 1));
			else
				host = default_host, port = static_cast<unsigned short>(atoi(address));
		}


		template <typename T>
		inline socket_handle::socket_handle(T s)
			: _socket(0)
		{	reset(static_cast<int>(s));	}

		inline socket_handle::socket_handle(socket_handle &other)
			: _socket(other._socket)
		{	other._socket = 0;	}

		inline socket_handle::~socket_handle()
		{	reset();	}

		inline void socket_handle::reset() throw()
		{
			if (_socket)
				::close(_socket);
			_socket = 0;
		}

		inline void socket_handle::reset(int s)
		{
			if (s == -1)
				throw std::invalid_argument("invalid socket");
			reset();
			_socket = s;
		}

		inline socket_handle::operator int() const
		{	return _socket;	}


		inline sockaddr_in make_sockaddr_in(const char *ip_address, unsigned short port)
		{
			sockaddr_in addr = {};

			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = inet_addr(ip_address);
			addr.sin_port = htons(port);
			return addr;
		}
	}
}
