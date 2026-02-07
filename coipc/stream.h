#pragma once

#include "noncopyable.h"
#include "range.h"
#include "types.h"

#include <stdexcept>

namespace coipc
{
	struct insufficient_buffer_error : std::runtime_error
	{
		insufficient_buffer_error(std::size_t requested_, std::size_t available_);

		std::size_t requested, available;
	};

	class buffer_reader : noncopyable
	{
	public:
		buffer_reader(const_byte_range payload);

		void read(void *data, std::size_t size);
		void skip(std::size_t size);

	private:
		void raise(std::size_t size);

	private:
		const byte *_ptr;
		std::size_t _remaining;
	};

	template <typename BufferT>
	class buffer_writer : noncopyable
	{
	public:
		buffer_writer(BufferT &buffer);

		void write(const void *data, std::size_t size);

	private:
		BufferT &_buffer;
	};



	template <typename BufferT>
	inline buffer_writer<BufferT>::buffer_writer(BufferT &buffer)
		: _buffer(buffer)
	{	_buffer.clear();	}

	template <typename BufferT>
	inline void buffer_writer<BufferT>::write(const void *data, std::size_t size)
	{
		const auto data_ = static_cast<const byte *>(data);

		if (size == 1)
			_buffer.push_back(*data_);
		else
			_buffer.insert(_buffer.end(), data_, data_ + size);
	}
}
