#include "helpers.h"

#include <algorithm>
#include <chrono>
#include <time.h>

#ifdef _WIN32
	#include <fcntl.h>
	#include <io.h>
#else
	#include <unistd.h>
#endif

using namespace std;
using namespace chrono;

namespace coipc
{
	namespace tests
	{
		stopwatch::stopwatch()
			: _period(1e-9), _last(duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count())
		{	}

		double stopwatch::operator ()() throw()
		{
			const auto current = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
			const auto elapsed = _period * (current - _last);

			_last = current;
			return elapsed;
		}


		guid_t generate_id()
		{
			static bool seed_initialized = false;

			if (!seed_initialized)
				srand(static_cast<unsigned int>(clock())), seed_initialized = true;

			guid_t id;

			generate(begin(id.values), end(id.values), &rand);
			return id;
		}

		tuple<shared_ptr<FILE>, shared_ptr<FILE>> create_pipe()
		{
			typedef shared_ptr<FILE> file_ptr;
			int fds[2];

#ifdef _WIN32
			if (!_pipe(fds, 4096, _O_BINARY))
				return make_tuple(file_ptr(_fdopen(fds[0], "rb"), &fclose), file_ptr(_fdopen(fds[1], "wb"), &fclose));
#else
			if (!pipe(fds) != 0)
				return make_tuple(file_ptr(fdopen(fds[0], "r"), &fclose), file_ptr(fdopen(fds[1], "w"), &fclose));
#endif
			throw runtime_error("_pipe() failed");
		}
	}
}
