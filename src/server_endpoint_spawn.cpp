#include <coipc/endpoint_spawn.h>

#include "helpers.h"

#ifdef WIN32
	#include <fcntl.h>
	#include <io.h>
#endif

using namespace coipc;
using namespace std;

int main(int argc, const char *argv[])
{
	class pipe_channel : public channel
	{
	public:
		pipe_channel(FILE *stream)
			: _stream(stream)
		{	}

	private:
		virtual void disconnect() throw()
		{	}

		virtual void message(const_byte_range payload)
		{
			auto size = static_cast<unsigned int>(payload.length());

			fwrite(&size, sizeof size, 1, _stream);
			fwrite(payload.begin(), 1, payload.length(), _stream);
			fflush(_stream);
		}

	private:
		FILE *const _stream;
	};

#ifdef WIN32
	_setmode(_fileno(stdin), O_BINARY);
	_setmode(_fileno(stdout), O_BINARY);
#endif

	vector<string> arguments;
	pipe_channel outbound(stdout);

	for (auto i = 1; i != argc; ++i)
		arguments.push_back(argv[i]);

	const auto instance = spawn::create_session(arguments, outbound);

	read_messages(*instance, [] (void *buffer, size_t size) {
		return fread(buffer, 1, size, stdin);
	}, false);
	instance->disconnect();
	return 0;
}
