#pragma once

#include <coipc/noncopyable.h>

namespace coipc
{
	namespace com
	{
		struct com_initialize : noncopyable
		{
			com_initialize();
			~com_initialize();
		};
	}
}
