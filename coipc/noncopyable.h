#pragma once

namespace coipc
{
	class noncopyable
	{
		noncopyable(const noncopyable &other);
		const noncopyable &operator =(const noncopyable &rhs);

	protected:
		noncopyable() {	}
	};
}
