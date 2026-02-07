#include <coipc/client_session.h>

#include <limits>
#include <numeric>

using namespace std;

namespace coipc
{
	client_session::client_session(channel &outbound)
		: _token(1), _callbacks(make_shared<callbacks_t>()), _message_callbacks(make_shared<message_callbacks_t>()),
			_outbound(&outbound)
	{	}

	client_session::~client_session()
	{	}

	void client_session::disconnect_session() throw()
	{	_outbound->disconnect();	}

	void client_session::disconnect() throw()
	{
	}

	void client_session::message(const_byte_range payload)
	{
		buffer_reader r(payload);
		deserializer d(r);
		int response_id;

		d(response_id);

		auto m = _message_callbacks->find(response_id);

		if (_message_callbacks->end() != m)
		{
			m->second(d);
		}
		else if (_callbacks->lower_bound(make_pair(response_id, numeric_limits<token_t>::min()))
			!= _callbacks->upper_bound(make_pair(response_id, numeric_limits<token_t>::max())))
		{
			token_t token;

			d(token);

			auto i = _callbacks->find(make_pair(response_id, token));

			if (_callbacks->end() != i)
				i->second(d);
		}
	}
}
