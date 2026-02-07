#pragma once

#include <string>

namespace coipc
{
	struct normalize
	{
		static std::string exe(const std::string &path);
		static std::string lib(const std::string &path);
	};



	std::string operator &(const std::string &lhs, const std::string &rhs); // path combine
	std::string operator ~(const std::string &value); // directory
	const char *operator *(const std::string &value); // filename
}
