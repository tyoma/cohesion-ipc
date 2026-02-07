#pragma once

#include <coipc/endpoint.h>

#include <functional>
#include <mt/mutex.h>
#include <vector>

namespace coipc
{
	namespace tests
	{
		namespace mocks
		{
			class channel : public coipc::channel
			{
			public:
				std::function<void (const_byte_range payload)> on_message;
				std::function<void ()> on_disconnect;

			private:
				virtual void disconnect() throw();
				virtual void message(const_byte_range payload);
			};

			class session : public coipc::channel
			{
			public:
				session();

			public:
				coipc::channel *outbound;
				std::vector< std::vector<std::uint8_t> > payloads_log;
				unsigned disconnections;
				std::function<void()> received_message;
				std::function<void()> disconnected;

			private:
				virtual void disconnect() throw();
				virtual void message(const_byte_range payload);
			};

			class server : public coipc::server
			{
			public:
				std::vector< std::shared_ptr<session> > sessions;
				std::function<void (const std::shared_ptr<session> &new_session)> session_created;

			private:
				virtual channel_ptr_t create_session(coipc::channel &outbound);

			private:
				mt::mutex _mutex;
			};



			inline void channel::disconnect() throw()
			{
				if (on_disconnect)
					on_disconnect();
			}

			inline void channel::message(const_byte_range payload)
			{
				if (on_message)
					on_message(payload);
			}


			inline session::session()
				: disconnections(0)
			{	}

			inline void session::disconnect() throw()
			{
				disconnections++;
				if (disconnected)
					disconnected();
			}

			inline void session::message(const_byte_range payload)
			{
				payloads_log.push_back(std::vector<std::uint8_t>(payload.begin(), payload.end()));
				if (received_message)
					received_message();
			}


			inline channel_ptr_t server::create_session(coipc::channel &outbound)
			{
				std::shared_ptr<session> s(new session);
				mt::lock_guard<mt::mutex> lock(_mutex);

				s->outbound = &outbound;
				sessions.push_back(s);
				if (session_created)
					session_created(s);
				return s;
			}
		}
	}
}
