#pragma once

typedef struct _GUID GUID;

namespace coipc
{
	typedef unsigned char byte;

#pragma pack(push, 1)
	struct guid_t
	{
		byte values[16];

		operator const GUID &() const;
	};
#pragma pack(pop)



	inline guid_t::operator const GUID &() const
	{	return *reinterpret_cast<const GUID *>(values);	}
}
