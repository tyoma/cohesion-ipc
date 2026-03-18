#pragma once

#include "../endpoint_spawn.h"

#include <mt/thread.h>
#include <stdio.h>

namespace coipc
{
	namespace spawn
	{
		class client_session : public channel
		{
		public:
			client_session(const std::string &spawned_path, const std::vector<std::string> &arguments,
				const std::vector<std::string> &extra_environment, channel &inbound);
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
				const std::vector<std::string> &extra_environment);

		private:
			virtual void disconnect() throw() override;
			virtual void message(const_byte_range payload) override;

		private:
			std::unique_ptr<mt::thread> _thread;
			std::shared_ptr<FILE> _outbound;
		};
	}
}
