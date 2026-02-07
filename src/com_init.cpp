#include <coipc/com/init.h>

#include <windows.h>

namespace coipc
{
	namespace com
	{
		com_initialize::com_initialize()
		{
			// TODO: need to check apartment model and fail if impossible to initialize with MT-model.
			::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		}

		com_initialize::~com_initialize()
		{
			::CoUninitialize();
		}
	}
}
