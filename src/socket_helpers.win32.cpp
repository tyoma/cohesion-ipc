#include "socket_helpers.h"

#include <windows.h>

namespace coipc
{
	namespace sockets
	{
		sockets_initializer::sockets_initializer()
		{
			WSADATA data = { };
			::WSAStartup(MAKEWORD(2, 2), &data);
		}

		sockets_initializer::~sockets_initializer()
		{	::WSACleanup();	}
	}
}
