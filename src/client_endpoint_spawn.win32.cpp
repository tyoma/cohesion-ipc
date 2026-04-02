#include <coipc/spawn/endpoint.h>

#include <coipc/exceptions.h>

#include <io.h>
#include <windows.h>

using namespace std;

namespace coipc
{
	namespace spawn
	{
		namespace
		{
			enum {
				creation_flags = CREATE_UNICODE_ENVIRONMENT | CREATE_NO_WINDOW,
			};

			shared_ptr<FILE> from_handle(HANDLE handle, const char *mode)
			{
				auto fd = _open_osfhandle(reinterpret_cast<intptr_t>(handle), 0);

				if (fd < 0)
					throw runtime_error("crt error");
				else if (auto stream = _fdopen(fd, mode))
					return shared_ptr<FILE>(stream, &fclose);
				_close(fd);
				throw runtime_error("crt stream error");
			}

			vector<wchar_t> initialize_environment(const vector<string> &extra_environment)
			{
				vector<wchar_t> merged;
				size_t length;

				for (auto e = ::GetEnvironmentStringsW(); (length = wcslen(e)); e += length + 1)
					merged.insert(merged.end(), e, e + length + 1);
				for (auto &e : extra_environment)
					merged.insert(merged.end(), e.c_str(), e.c_str() + e.size() + 1);
				merged.push_back(0);
				return merged;
			}

			template <typename T>
			void append_quoted(vector<wchar_t> &cmdl, const T &part)
			{
				cmdl.push_back('\"');
				cmdl.insert(cmdl.end(), part.begin(), part.end());
				cmdl.push_back('\"');
				cmdl.push_back(' ');
			}

			wstring unicode(const string &s)
			{
				auto required = ::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.length()), NULL, 0);

				if (required <= 0)
					throw runtime_error("MultiByteToWideChar failed");
				vector<wchar_t> buffer(required);
				if (!::MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.length()), buffer.data(), required))
					throw runtime_error("MultiByteToWideChar failed");
				return wstring(buffer.data(), buffer.size());
			}
		}

		client_session::spawned client_session::spawn(const string &spawned_path, const vector<string> &arguments,
			const vector<string> &extra_environment, exit_handler_t &&exit_handler)
		{
			STARTUPINFOW si = {};
			PROCESS_INFORMATION process = {};
			vector<wchar_t> cmdl;
			auto env = initialize_environment(extra_environment);
			HANDLE hpipes[2];

			si.cb = sizeof si;
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdError = INVALID_HANDLE_VALUE;

			if (!::CreatePipe(&hpipes[0], &hpipes[1], NULL, 0))
				throw bad_alloc();

			auto stdin_r = from_handle(si.hStdInput = hpipes[0], "rb");
			auto stdin_w = from_handle(hpipes[1], "wb");

			if (!::CreatePipe(&hpipes[0], &hpipes[1], NULL, 0))
				throw bad_alloc();

			auto stdout_r = from_handle(hpipes[0], "rb");
			auto stdout_w = from_handle(si.hStdOutput = hpipes[1], "wb");

			::SetHandleInformation(si.hStdInput, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
			::SetHandleInformation(si.hStdOutput, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

			append_quoted(cmdl, unicode(spawned_path));
			for (auto i = arguments.begin(); i != arguments.end(); ++i)
				append_quoted(cmdl, unicode(*i));
			cmdl.back() = 0;
			if (!::CreateProcessW(NULL, cmdl.data(), NULL, NULL, TRUE, creation_flags, env.data(), NULL, &si, &process))
				throw server_exe_not_found(("Server executable not found: " + spawned_path).c_str());

			::CloseHandle(process.hThread);
			return spawned {
				stdin_w, stdout_r, shared_ptr<void>(process.hProcess, [exit_handler] (void *hprocess) {
					DWORD exit_code;

					::WaitForSingleObject(hprocess, INFINITE);
					::GetExitCodeProcess(hprocess, &exit_code);
					exit_handler(static_cast<int>(exit_code));
					::CloseHandle(hprocess);
				})
			};
		}
	}
}
