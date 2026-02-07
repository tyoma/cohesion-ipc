#include <dlfcn.h>
#include <mach/vm_map.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>
#include <vector>

using namespace std;

namespace coipc
{
	namespace tests
	{
		string executable()
		{
			char dummy;
			uint32_t l = 0;

			::_NSGetExecutablePath(&dummy, &l);

			vector<char> buffer(l);

			::_NSGetExecutablePath(&buffer[0], &l);
			buffer.push_back('\0');
			return &buffer[0];
		}

		string get_this_module_path()
		{
			Dl_info di = { };

			::dladdr((void*)executable, &di);
			return di.dli_fname && *di.dli_fname ? di.dli_fname : executable();
		}
	}
}
