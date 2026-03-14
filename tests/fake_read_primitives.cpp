#include "fake_read_primitives.h"

#include <cstring>

namespace coipc
{
	namespace tests
	{
		fake_read_primitives::fake_read_primitives()
			: _read_calls(0), _cancel_calls(0), _entered(0), _about_to_engage(0), _block_read(0),
				_pause_before_engage(0), _cancel_failures_left(0)
		{	}

		size_t fake_read_primitives::read_calls() const
		{	return _read_calls.load(std::memory_order_acquire);	}

		size_t fake_read_primitives::cancel_calls() const
		{	return _cancel_calls.load(std::memory_order_acquire);	}

		void fake_read_primitives::block_next_read()
		{	_block_read.store(1, std::memory_order_release);	}

		void fake_read_primitives::unblock_read()
		{
			{
				std::lock_guard<std::mutex> l(_mtx);
				_block_read.store(0, std::memory_order_release);
			}
			_cv.notify_all();
		}

		void fake_read_primitives::pause_before_engage()
		{	_pause_before_engage.store(1, std::memory_order_release);	}

		void fake_read_primitives::allow_engage()
		{
			{
				std::lock_guard<std::mutex> l(_mtx);
				_pause_before_engage.store(0, std::memory_order_release);
			}
			_cv.notify_all();
		}

		void fake_read_primitives::wait_until_read_entered()
		{
			std::unique_lock<std::mutex> l(_mtx);
			_cv.wait(l, [&] { return !!_entered.load(std::memory_order_acquire); });
		}

		void fake_read_primitives::wait_until_about_to_engage()
		{
			std::unique_lock<std::mutex> l(_mtx);
			_cv.wait(l, [&] { return !!_about_to_engage.load(std::memory_order_acquire); });
		}

		void fake_read_primitives::set_cancel_plan(unsigned failures_before_success)
		{	_cancel_failures_left.store(failures_before_success, std::memory_order_release);	}

		void fake_read_primitives::set_flag_and_notify(std::atomic<int> &flag)
		{
			{
				std::lock_guard<std::mutex> l(_mtx);
				flag.store(1, std::memory_order_release);
			}
			_cv.notify_all();
		}

		std::size_t fake_read_primitives::read(std::FILE & /*file*/, void *buffer, std::size_t size)
		{
			_read_calls.fetch_add(1, std::memory_order_acq_rel);

			set_flag_and_notify(_entered);

			set_flag_and_notify(_about_to_engage);

			{
				std::unique_lock<std::mutex> l(_mtx);
				_cv.wait(l, [&] {
					return !_pause_before_engage.load(std::memory_order_acquire)
						&& !_block_read.load(std::memory_order_acquire);
				});
			}

			// Make it look like we "read" something.
			const size_t n = size ? 1 : 0;
			if (n)
				std::memset(buffer, 0, 1);
			return n;
		}

		bool fake_read_primitives::cancel_io(void * /*thread_handle*/)
		{
			_cancel_calls.fetch_add(1, std::memory_order_acq_rel);

			unsigned left = _cancel_failures_left.load(std::memory_order_acquire);
			if (left)
			{
				_cancel_failures_left.store(left - 1, std::memory_order_release);
				return false;
			}
			return true;
		}
	}
}