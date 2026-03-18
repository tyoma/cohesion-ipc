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

			static std::pair<std::shared_ptr<FILE> /*outbound*/, std::shared_ptr<FILE> /*inbound*/> spawn(
				const std::string &spawned_path, const std::vector<std::string> &arguments,
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
