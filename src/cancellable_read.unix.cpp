#include <coipc/cancellable_read.h>

#include <mutex>

#include <errno.h>
#include <sys/select.h>
#include <unistd.h>

using namespace std;

namespace coipc
{
	namespace
	{
		struct unix_platform : cancellable_read::platform
		{
			virtual size_t read(FILE &file, void *buffer, size_t size) override
			{	return ::fread(buffer, 1, size, &file);	}

			virtual bool cancel_io(void * /*thread_handle*/) override
			{	return true;	}
		};

		unix_platform g_default_platform;
	}

	cancellable_read::platform &cancellable_read::default_implementation = g_default_platform;


	class cancellable_read::impl
	{
	public:
		impl(platform & /*platform_*/)
			: _cancelled(false)
		{
			if (::pipe(_signal_pipe) < 0)
				throw runtime_error("pipe() failed");
		}

		~impl()
		{
			::close(_signal_pipe[0]);
			::close(_signal_pipe[1]);
		}

		size_t read(FILE &file, void *buffer, size_t size)
		{
			auto data_fd = ::fileno(&file);

			for (;;)
			{
				fd_set rfds;
				FD_ZERO(&rfds);
				FD_SET(data_fd, &rfds);
				FD_SET(_signal_pipe[0], &rfds);

				auto nfds = (data_fd > _signal_pipe[0] ? data_fd : _signal_pipe[0]) + 1;
				auto ready = ::select(nfds, &rfds, nullptr, nullptr, nullptr);

				if (ready < 0 && errno == EINTR)
					continue;
				if (ready < 0)
					throw runtime_error("select() failed");

				if (FD_ISSET(_signal_pipe[0], &rfds))
					throw cancelled_exception();

				if (FD_ISSET(data_fd, &rfds))
					return ::fread(buffer, 1, size, &file);
			}
		}

		void cancel()
		{	::write(_signal_pipe[1], _signal_pipe, 1);	}

	private:
		impl(const impl &);
		const impl &operator =(const impl &);

	private:
		mutex _mtx;
		bool _cancelled;
		int _signal_pipe[2];
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
