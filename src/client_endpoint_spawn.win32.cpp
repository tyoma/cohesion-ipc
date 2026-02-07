#include <coipc/spawn/endpoint.h>

#include <io.h>
#include <windows.h>

using namespace std;

namespace coipc
{
	namespace spawn
	{
		namespace
		{
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

			template <typename T>
			void append_quoted(vector<wchar_t> &command_line, const T &part)
			{
				command_line.push_back('\"');
				command_line.insert(command_line.end(), part.begin(), part.end());
				command_line.push_back('\"');
				command_line.push_back(' ');
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

		pair< shared_ptr<FILE>, shared_ptr<FILE> > client_session::spawn(const string &spawned_path,
			const vector<string> &arguments)
		{
			STARTUPINFOW si = {};
			PROCESS_INFORMATION process = {};
			vector<wchar_t> command_line;
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

			append_quoted(command_line, unicode(spawned_path));
			for (auto i = arguments.begin(); i != arguments.end(); ++i)
				append_quoted(command_line, unicode(*i));
			command_line.back() = 0;
			if (!::CreateProcessW(NULL, command_line.data(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &process))
				throw server_exe_not_found(("Server executable not found: " + spawned_path).c_str());

			::CloseHandle(process.hProcess);
			::CloseHandle(process.hThread);
			return make_pair(stdin_w, stdout_r);
		}
	}
}
