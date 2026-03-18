#pragma once

#include <coipc/endpoint.h>
#include <coipc/exceptions.h>

#include <algorithm>
#include <string>

namespace coipc
{
	template <typename FactoryT>
	struct constructor
	{
		std::string protocol;
		FactoryT constructor_method;
	};


	template <typename T>
	union byte_representation
	{
		char bytes[sizeof(T)];
		T value;

		void reorder();
	};



	template <typename FactoryT, size_t n>
	inline const FactoryT &select(constructor<FactoryT> (&constructors)[n], const std::string &typed_endpoint_id,
		std::string &endpoint_id)
	{
		const auto delim = typed_endpoint_id.find('|');
		if (delim == std::string::npos)
			throw std::invalid_argument(typed_endpoint_id);
		const auto protocol = typed_endpoint_id.substr(0, delim);

		endpoint_id = typed_endpoint_id.substr(delim + 1);

		for (auto i = 0u; i != n; ++i)
		{
			if (protocol == constructors[i].protocol)
				return constructors[i].constructor_method;
		}
		throw protocol_not_supported(typed_endpoint_id.c_str());
	}


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


	template <typename ReadT>
	inline void read_messages(coipc::channel &inbound, ReadT &&read_function, bool reorder)
	{
		byte_representation<unsigned int> size;

		for (std::vector<std::uint8_t> buffer; read_function(size.bytes, sizeof(size)) == sizeof(size); )
		{
			if (reorder)
				size.reorder();
			buffer.resize(size.value);
			read_function(buffer.data(), size.value);
			inbound.message(const_byte_range(buffer.data(), buffer.size()));
		}
	}
}
