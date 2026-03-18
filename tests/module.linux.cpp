#include <dlfcn.h>
#include <link.h>
#include <signal.h>
#include <stdexcept>
#include <unistd.h>

using namespace std;

namespace coipc
{
	namespace tests
	{
		bool is_process_running(unsigned int pid)
		{	return ::kill(pid, 0) == 0;	}

		string executable()
		{
			char path[1000];
			int result = ::readlink("/proc/self/exe", path, sizeof(path) - 1);

			return path[result >= 0 ? result : 0] = 0, path;
		}

		string get_this_module_path()
		{
			Dl_info di = { };

			::dladdr((void*)executable, &di);
			return di.dli_fname && *di.dli_fname ? di.dli_fname : executable();
		}
	}
}
