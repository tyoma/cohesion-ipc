#pragma once

#include "endpoint.h"
#include "serialization.h"

#include <functional>
#include <map>

namespace coipc
{
	class client_session : public channel
	{
	public:
		typedef std::function<void (deserializer &payload_deserializer)> callback_t;

	public:
		// User establishes and controls the connection.
		template <typename ChannelFactoryT>
		client_session(const ChannelFactoryT &connection_factory);

		// Connection is established and controlled by an outside entity.
		client_session(channel &outbound);

		virtual ~client_session();

		void disconnect_session() throw();

		template <typename MessageCallbackT>
		void subscribe(std::shared_ptr<void> &handle, int message_id, const MessageCallbackT &callback);

		template <typename RequestT, typename ResponseCallbackT>
		void request(std::shared_ptr<void> &handle, int id, const RequestT &payload, int response_id,
			const ResponseCallbackT &callback);

		template <typename RequestT, typename MultiResponseCallbackT>
		void request(std::shared_ptr<void> &handle, int id, const RequestT &payload,
			const MultiResponseCallbackT &callback);

		// channel methods
		virtual void disconnect() throw() override;
		virtual void message(const_byte_range payload) override;

	private:
		typedef unsigned long long token_t;
		typedef std::map<std::pair<int, token_t>, callback_t> callbacks_t;
		typedef std::map<int, callback_t> message_callbacks_t;

	private:
		template <typename RequestT, typename CallbackConstructorT>
		void request_internal(int id, const RequestT &payload, const CallbackConstructorT &callback_ctor);

	private:
		std::vector<std::uint8_t> _buffer;
		token_t _token;
		std::shared_ptr<callbacks_t> _callbacks;
		std::shared_ptr<message_callbacks_t> _message_callbacks;
		channel_ptr_t _outbound_active;
		channel *_outbound;
	};



	template <typename ChannelFactoryT>
	inline client_session::client_session(const ChannelFactoryT &connection_factory)
		: _token(1), _callbacks(std::make_shared<callbacks_t>()),
			_message_callbacks(std::make_shared<message_callbacks_t>())
	{	_outbound = (_outbound_active = connection_factory(*this)).get();	}

	template <typename MessageCallbackT>
	inline void client_session::subscribe(std::shared_ptr<void> &handle, int message_id,
		const MessageCallbackT &callback)
	{
		auto callbacks = _message_callbacks;
		auto i = callbacks->insert(std::make_pair(message_id, callback)).first;

		handle.reset(&*i, [callbacks, i] (...) {	callbacks->erase(i);	});
	}

	template <typename RequestT, typename ResponseCallbackT>
	inline void client_session::request(std::shared_ptr<void> &handle, int id, const RequestT &payload,
		int response_id, const ResponseCallbackT &callback)
	{
		request_internal(id, payload, [&] (token_t token) {
			auto callbacks = _callbacks;
			auto i = callbacks->insert(std::make_pair(std::make_pair(response_id, token), callback)).first;

			handle.reset(&*i, [callbacks, i] (...) {	callbacks->erase(i);	});
		});
	}

	template <typename RequestT, typename MultiResponseCallbackT>
	inline void client_session::request(std::shared_ptr<void> &handle, int id, const RequestT &payload,
		const MultiResponseCallbackT &callback)
	{
		request_internal(id, payload, [&] (token_t token) {
			auto callbacks = _callbacks;

			for (auto i = std::begin(callback); i != std::end(callback); ++i)
			{
				auto j = callbacks->insert(std::make_pair(std::make_pair(i->first, token), i->second)).first;

				if (handle)
				{
					auto h2 = handle;

					handle = std::shared_ptr<void>(&*j, [h2, callbacks, j] (...) {	callbacks->erase(j);	});
					continue;
				}
				handle.reset(&*j, [callbacks, j] (...) {	callbacks->erase(j);	});
			}
		});
	}

	template <typename RequestT, typename CallbackConstructorT>
	inline void client_session::request_internal(int id, const RequestT &payload,
		const CallbackConstructorT &callback_ctor)
	{
		_buffer.clear();
		buffer_writer< std::vector<std::uint8_t> > w(_buffer);
		coipc::serializer s(w);
		auto token = _token++;

		s(id);
		s(token);
		s(payload);
		callback_ctor(token);
		_outbound->message(const_byte_range(_buffer.data(), _buffer.size()));
	}
}
