#include <string>
#include <windows.h>

using namespace std;

namespace coipc
{
	namespace tests
	{
		string get_this_module_path()
		{
			enum {	length = MAX_PATH,	};
			char buffer[length + 1] = { 0 };
			HMODULE h = NULL;

			::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)(void *)&get_this_module_path, &h);
			::GetModuleFileNameA(h, buffer, length);
			return buffer;
		}
	}
}
