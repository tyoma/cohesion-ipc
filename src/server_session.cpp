#include <coipc/server_session.h>

using namespace std;

namespace coipc
{
	server_session::server_session(channel &outbound, tasker::queue *apartment)
		: _outbound(outbound), _apartment_queue(apartment ? new tasker::private_queue(*apartment) : nullptr)
	{	}

	void server_session::set_disconnect_handler(const function<void ()> &handler)
	{	_disconnect_handler = handler;	}

	void server_session::disconnect() throw()
	{
		if (_disconnect_handler)
			_disconnect_handler();
	}

	void server_session::message(const_byte_range payload)
	{
		buffer_reader r(payload);
		deserializer d(r);
		int request_id;
		token_t token;

		d(request_id);
		d(token);

		const auto h = _handlers.find(request_id);

		if (h != _handlers.end())
		{
			response resp(*this, token, !!_apartment_queue.get());

			h->second(resp, d);
			if (resp.continuation)
				schedule_continuation(token, resp.continuation);
		}
	}

	void server_session::schedule_continuation(token_t token,
		const function<void (response &response_)> &continuation_handler)
	{
		_apartment_queue->schedule([this, token, continuation_handler] {
			response resp(*this, token, true);

			continuation_handler(resp);
			if (resp.continuation)
				schedule_continuation(token, resp.continuation);
		});
	}


	server_session::response::response(server_session &owner, token_t token, bool deferral_enabled)
		: _owner(owner), _token(token), _deferral_enabled(deferral_enabled)
	{	}

	void server_session::response::defer(const function<void (response &response_)> &continuation_handler)
	{
		if (!_deferral_enabled)
			throw logic_error("deferring is disabled - no queue");
		continuation = continuation_handler;
	}
}
