#pragma once

#include <string>
#include <stdexcept>

namespace coipc
{
	template <typename FactoryT>
	struct constructor
	{
		std::string protocol;
		FactoryT constructor_method;
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
}
