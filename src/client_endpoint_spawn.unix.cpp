#include <coipc/spawn/endpoint.h>

#include <memory>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

extern char **environ;

namespace coipc
{
	namespace spawn
	{
		namespace
		{
			shared_ptr<FILE> from_fd(int fd, const char *mode)
			{
				if (auto stream = fdopen(fd, mode))
					return shared_ptr<FILE>(stream, &fclose);
				::close(fd);
				throw runtime_error("crt stream error");
			}
		}

		client_session::spawned client_session::spawn(const string &spawned_path, const vector<string> &arguments,
			const vector<string> &extra_environment, exit_handler_t &&exit_handler)
		{
			// pipes[0]: parent writes, child reads (child's stdin)
			// pipes[1]: child writes, parent reads (child's stdout)
			int pipes[2][2];

			if (::pipe(pipes[0]) < 0)
				throw bad_alloc();
			if (::pipe(pipes[1]) < 0)
			{
				::close(pipes[0][0]), ::close(pipes[0][1]);
				throw bad_alloc();
			}

			const auto pid = ::fork();

			switch (pid)
			{
			default:
				// parent
				::close(pipes[0][0]), ::close(pipes[1][1]);
				return spawned {
					from_fd(pipes[0][1], "w"), from_fd(pipes[1][0], "r"), shared_ptr<void>(nullptr, [exit_handler, pid] (void *) {
						siginfo_t si = {};

						::waitid(P_PID, pid, &si, WEXITED);
						exit_handler(si.si_status);
					}),
				};

			case -1:
				// Forking error...
				throw runtime_error("forking the process failed");

			case 0:
				// child
				::close(pipes[0][1]), ::close(pipes[1][0]);
				::dup2(pipes[0][0], STDIN_FILENO), ::dup2(pipes[1][1], STDOUT_FILENO);

				auto spawned_path_ = spawned_path;
				auto arguments_ = arguments;
				auto extra_environment_ = extra_environment;
				vector<char *> argv, env;

				argv.push_back(&spawned_path_[0]);
				for (auto &i : arguments_)
					argv.push_back(&i[0]);
				argv.push_back(nullptr);
				for (auto e = environ; *e; ++e)
					env.push_back(*e);
				for (auto &i : extra_environment_)
					env.push_back(&i[0]);
				env.push_back(nullptr);
				if (::execve(spawned_path.c_str(), argv.data(), env.data()) < 0)
					exit(-1);
				throw 0;
			}
		}
	}
}
