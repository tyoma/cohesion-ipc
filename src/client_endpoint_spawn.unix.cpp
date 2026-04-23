#include <coipc/spawn/endpoint.h>

#include <coipc/exceptions.h>

#include <fcntl.h>
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
		client_session::spawned client_session::spawn(const string &spawned_path, const vector<string> &arguments,
			const vector<string> &extra_environment, exit_handler_t &&exit_handler)
		{
			struct anonymous_pipe
			{
				anonymous_pipe()
				{
					int p[2];
					auto from_fd = [] (int fd, const char *mode) {
						auto stream = fdopen(fd, mode);
						return stream ? shared_ptr<FILE>(stream, &fclose) : (::close(fd), nullptr);
					};

					if (::pipe(p) < 0)
						throw bad_alloc();
					read = from_fd(p[0], "r");
					write = from_fd(p[1], "w");
					if (!read || !write)
						throw runtime_error("crt stream error");
				}

				shared_ptr<FILE> read, write;
			} to, from, from_error;

			fcntl(fileno(from_error.write.get()), F_SETFD, FD_CLOEXEC);

			const auto pid = ::fork();
			int exec_error;

			switch (pid)
			{
			default:
				// parent
				from_error.write.reset();
				if (1 == fread(&exec_error, sizeof(exec_error), 1, from_error.read.get()))
					throw server_exe_not_found(("Failed to execute: " + spawned_path + ", error: " + to_string(exec_error)).c_str());
				return spawned {
					to.write, from.read, shared_ptr<void>(nullptr, [exit_handler, pid] (void *) {
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
				to.write.reset(), from.read.reset(), from_error.read.reset();
				::dup2(fileno(to.read.get()), STDIN_FILENO), ::dup2(fileno(from.write.get()), STDOUT_FILENO);

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
				::execve(spawned_path.c_str(), argv.data(), env.data());
				exec_error = errno;
				fwrite(&exec_error, sizeof(exec_error), 1, from_error.write.get());
				exit(-1);
			}
		}
	}
}
