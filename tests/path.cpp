#include "path.h"

using namespace std;

namespace coipc
{
	string operator &(const string &lhs, const string &rhs)
	{
		if (lhs.empty())
			return rhs;
		if (lhs.back() == '\\' || lhs.back() =='/')
			return lhs + rhs;
		return lhs + '/' + rhs;
	}

	string operator ~(const string &value)
	{
		const char separators[] = {	'\\', '/', '\0'	};
		const auto pos = value.find_last_of(separators);

		if (pos != string::npos)
			return value.substr(0, pos);
		return string();
	}

	const char *operator *(const string &value)
	{
		const char separators[] = {	'\\', '/', '\0'	};
		const auto pos = value.find_last_of(separators);

		return value.c_str() + (pos != string::npos ? pos + 1 : 0u);
	}


	string normalize::exe(const string &path)
	{
#if defined(_WIN32)
		return path + ".exe";
#else
		return path;
#endif
	}

	string normalize::lib(const string &path)
	{
#if defined(_WIN32)
		return path + ".dll";
#elif defined(__APPLE__)
		return ~path & (string("lib") + *path + ".dylib");
#else
		return ~path & (string("lib") + *path + ".so");
#endif
	}
}
