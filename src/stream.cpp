#include <coipc/stream.h>

namespace coipc
{
	insufficient_buffer_error::insufficient_buffer_error(size_t requested_, size_t available_)
		: std::runtime_error("buffer is smaller than requested to read"), requested(requested_), available(available_)
	{	}



	buffer_reader::buffer_reader(const_byte_range payload)
		: _ptr(payload.begin()), _remaining(payload.length())
	{	}

	void buffer_reader::read(void *data, size_t size)
	{
		if (size > _remaining)
			raise(size);

		auto src = _ptr;
		auto dest = static_cast<byte *>(data);

		for (auto n = size; n--; )
			*dest++ = *src++;

		_ptr = src;
		_remaining -= size;
	}

	void buffer_reader::skip(size_t size)
	{
		if (size > _remaining)
			raise(size);
		_ptr += size;
		_remaining -= size;
	}

	void buffer_reader::raise(size_t size)
	{	throw insufficient_buffer_error(size, _remaining);	}
}
