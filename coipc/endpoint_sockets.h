#pragma once

#include "endpoint.h"

#include <algorithm>
#include <functional>

namespace coipc
{
	namespace sockets
	{
		channel_ptr_t connect_client(const char *destination_endpoint_id, channel &inbound);
		std::shared_ptr<void> run_server(const char *endpoint_id, const std::shared_ptr<server> &factory);


		template <typename T>
		union byte_representation
		{
			char bytes[sizeof(T)];
			T value;

			void reorder();
		};



		template <typename T>
		inline void byte_representation<T>::reorder()
		{
			byte_representation<unsigned> order;

			order.value = 0xFF;
			if (order.bytes[0])
			{
				for (auto i = 0u; i < sizeof(T) / 2; ++i)
					std::swap(bytes[i], bytes[sizeof(T) - 1 - i]);
			}
		}
	}
}
