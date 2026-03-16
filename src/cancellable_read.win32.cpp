#include <coipc/cancellable_read.h>

#include <coipc/exceptions.h>

#include <atomic>
#include <mutex>
#include <windows.h>

using namespace std;

namespace coipc
{
	namespace
	{
		struct win32_platform : cancellable_read::platform
		{
			virtual size_t read(FILE &file, void *buffer, size_t size) override
			{	return ::fread(buffer, 1, size, &file);	}

			virtual bool cancel_io(void *thread_handle) override
			{	return !!::CancelSynchronousIo(thread_handle);	}
		};

		win32_platform g_default_platform;
	}

	cancellable_read::platform &cancellable_read::default_implementation = g_default_platform;


	class cancellable_read::impl
	{
	public:
		impl(platform &platform_)
			: _platform(platform_), _cancelled(false), _thread_handle(nullptr)
		{	}

		size_t read(FILE &file, void *buffer, size_t size)
		{
			shared_ptr<void> this_thread(::OpenThread(THREAD_ALL_ACCESS, FALSE, GetCurrentThreadId()), &::CloseHandle);

			{
				lock_guard<mutex> l(_mtx);

				if (_cancelled)
					throw cancelled_exception();
				_thread_handle = this_thread.get();
			}

			auto bytes_read = _platform.read(file, buffer, size);
			lock_guard<mutex> l(_mtx);

			_thread_handle = nullptr;
			if (_cancelled)
				throw cancelled_exception();
			return bytes_read;
		}

		void cancel()
		{
			for (;; YieldProcessor())
			{
				lock_guard<mutex> l(_mtx);
				
				_cancelled = true;
				if (!_thread_handle || _platform.cancel_io(_thread_handle))
					break;
			}
		}

	private:
		platform &_platform;
		mutex _mtx;
		bool _cancelled;
		void *_thread_handle;
	};


	cancellable_read::cancellable_read(platform &implementation)
		: _impl(new impl(implementation))
	{	}

	cancellable_read::~cancellable_read()
	{	}

	size_t cancellable_read::read(FILE &file, void *buffer, size_t size)
	{	return _impl->read(file, buffer, size);	}

	void cancellable_read::cancel()
	{	_impl->cancel();	}
}
