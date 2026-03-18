#include <string>
#include <windows.h>

using namespace std;

namespace coipc
{
	namespace tests
	{
		bool is_process_running(unsigned int pid)
		{
			auto hprocess = ::OpenProcess(SYNCHRONIZE, FALSE, pid);
			auto ret = ::WaitForSingleObject(hprocess, 0);

			::CloseHandle(hprocess);
			return ret == WAIT_TIMEOUT;
		}

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
