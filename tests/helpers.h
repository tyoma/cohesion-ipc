#pragma once

#include <coipc/endpoint.h>
#include <coipc/misc.h>
#include <coipc/stream.h>
#include <strmd/serializer.h>
#include <vector>

namespace coipc
{
	typedef long long timestamp_t;

	
	class stopwatch
	{
	public:
		stopwatch();

		double operator ()() throw();

	private:
		double _period;
		timestamp_t _last;
	};


	struct plural_
	{
		template <typename T>
		std::vector<T> operator +(const T &rhs) const
		{	return std::vector<T>(1, rhs);	}
	} const plural;

	template <typename T>
	inline std::vector<T> operator +(std::vector<T> lhs, const T &rhs)
	{	return lhs.push_back(rhs), lhs;	}
	
	inline void send_standard(coipc::channel &c, int id, unsigned long long token)
	{
		std::vector<std::uint8_t> data;
		buffer_writer< std::vector<std::uint8_t> > w(data);
		strmd::serializer<buffer_writer< std::vector<std::uint8_t> >, strmd::varint> s(w);

		s(id);
		s(token);
		c.message(const_byte_range(data.data(), data.size()));
	}

	template <typename PayloadT>
	inline void send_standard(coipc::channel &c, int id, unsigned long long token, const PayloadT &payload)
	{
		std::vector<std::uint8_t> data;
		buffer_writer< std::vector<std::uint8_t> > w(data);
		strmd::serializer<buffer_writer< std::vector<std::uint8_t> >, strmd::varint> s(w);

		s(id);
		s(token);
		s(payload);
		c.message(const_byte_range(data.data(), data.size()));
	}

	template <typename PayloadT>
	inline void send_message(coipc::channel &c, int id, const PayloadT &payload)
	{
		std::vector<std::uint8_t> data;
		buffer_writer< std::vector<std::uint8_t> > w(data);
		strmd::serializer<buffer_writer< std::vector<std::uint8_t> >, strmd::varint> s(w);

		s(id);
		s(payload);
		c.message(const_byte_range(data.data(), data.size()));
	}

	inline void send_message(coipc::channel &c, int id)
	{
		std::vector<std::uint8_t> data;
		buffer_writer< std::vector<std::uint8_t> > w(data);
		strmd::serializer<buffer_writer< std::vector<std::uint8_t> >, strmd::varint> s(w);

		s(id);
		c.message(const_byte_range(data.data(), data.size()));
	}
		
	guid_t generate_id();

	template <typename T, std::size_t size>
	inline std::vector<T> mkvector(T (&array_ptr)[size])
	{	return std::vector<T>(array_ptr, array_ptr + size);	}

	template <typename T, std::size_t size>
	inline range<T, std::size_t> mkrange(T (&array_ptr)[size])
	{	return range<T, std::size_t>(array_ptr, size);	}
}
