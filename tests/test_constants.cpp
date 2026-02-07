#include "constants.h"

#include "path.h"

using namespace std;

namespace coipc
{
	namespace tests
	{
		string get_this_module_path();

		const string constants::c_this_module = get_this_module_path();
		const string constants::c_guinea_ipc_spawn = ~c_this_module & normalize::exe("guinea_ipc_spawn");
		const string constants::c_guinea_ipc_spawn_server = ~c_this_module & normalize::exe("guinea_ipc_spawn_server");
	}
}
