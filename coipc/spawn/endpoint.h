#pragma once

#include "../endpoint.h"

#include <functional>
#include <mt/thread.h>
#include <stdio.h>

namespace coipc
{
	namespace spawn
	{
		class client_session : public channel
		{
		public:
			typedef std::function<void (int exit_code)> exit_handler_t;

		public:
			client_session(const std::string &spawned_path, const std::vector<std::string> &arguments,
				const std::vector<std::string> &extra_environment, channel &inbound, exit_handler_t &&exit_handler);
			~client_session();

		private:
			struct spawned
			{
				std::shared_ptr<FILE> to;
				std::shared_ptr<FILE> from;
				std::shared_ptr<void> wait_handle;
			};

		private:
			static spawned spawn(const std::string &spawned_path, const std::vector<std::string> &arguments,
				const std::vector<std::string> &extra_environment, exit_handler_t &&exit_handler);

		private:
			virtual void disconnect() throw() override;
			virtual void message(const_byte_range payload) override;

		private:
			std::unique_ptr<mt::thread> _thread;
			std::shared_ptr<FILE> _outbound;
		};
	}
}
