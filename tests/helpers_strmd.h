#pragma once

#include "helpers.h"

#include <coipc/strmd/stream.h>

namespace coipc
{
	namespace strmd
	{
		namespace tests
		{
			inline void send_standard(coipc::channel &c, int id, unsigned long long token)
			{
				std::vector<std::uint8_t> data;
				buffer_writer< std::vector<std::uint8_t> > w(data);
				serializer s(w);

				s(id);
				s(token);
				c.message(const_byte_range(data.data(), data.size()));
			}

			template <typename PayloadT>
			inline void send_standard(coipc::channel &c, int id, unsigned long long token, const PayloadT &payload)
			{
				std::vector<std::uint8_t> data;
				buffer_writer< std::vector<std::uint8_t> > w(data);
				serializer s(w);

				s(id);
				s(token);
				s(payload);
				c.message(const_byte_range(data.data(), data.size()));
			}

			template <typename PayloadT>
			inline void send_message(coipc::channel &c, int id, const PayloadT &payload)
			{
				std::vector<std::uint8_t> data;
				buffer_writer< std::vector<std::uint8_t> > w(data);
				serializer s(w);

				s(id);
				s(payload);
				c.message(const_byte_range(data.data(), data.size()));
			}

			inline void send_message(coipc::channel &c, int id)
			{
				std::vector<std::uint8_t> data;
				buffer_writer< std::vector<std::uint8_t> > w(data);
				serializer s(w);

				s(id);
				c.message(const_byte_range(data.data(), data.size()));
			}
		}
	}
}
