#include "helpers.h"

#include <algorithm>
#include <chrono>
#include <time.h>

using namespace std;
using namespace std::chrono;

namespace coipc
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
}
