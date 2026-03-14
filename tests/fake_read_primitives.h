#pragma once

#include <coipc/cancellable_read.h>

#include <atomic>
#include <cstddef>
#include <cstdio>
#include <mutex>
#include <condition_variable>

namespace coipc
{
	namespace tests
	{
		class fake_read_primitives : public cancellable_read::platform
		{
		public:
			fake_read_primitives();

		public:
			size_t read_calls() const;
			size_t cancel_calls() const;

			// Control read timing.
			void block_next_read();
			void unblock_read();

			void pause_before_engage();
			void allow_engage();

			void wait_until_read_entered();
			void wait_until_about_to_engage();

			// Control cancel timing/result.
			void set_cancel_plan(unsigned failures_before_success);

		public:
			// cancellable_read::platform overrides.
			virtual std::size_t read(std::FILE &file, void *buffer, std::size_t size) override;
			virtual bool cancel_io(void *thread_handle) override;

		private:
			void set_flag_and_notify(std::atomic<int> &flag);

		private:
			mutable std::mutex _mtx;
			std::condition_variable _cv;

			std::atomic<size_t> _read_calls;
			std::atomic<size_t> _cancel_calls;

			std::atomic<int> _entered;
			std::atomic<int> _about_to_engage;

			std::atomic<int> _block_read;
			std::atomic<int> _pause_before_engage;

			std::atomic<unsigned> _cancel_failures_left;
		};
	}
}