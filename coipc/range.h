#pragma once

#include <cstdint>
#include <vector>

namespace coipc
{
#pragma pack(push, 1)
	template <typename T, typename SizeT>
	class range
	{
	private:
		typedef T value_type;

	public:
		template <typename U>
		range(const range<U, SizeT> &u);
		range(T *start, SizeT length);

		T *begin() const;
		T *end() const;
		T *data() const;
		SizeT length() const;
		bool inside(const T *ptr) const;
		range prefix(SizeT length_) const;
		range suffix(SizeT start) const;

	private:
		T *_data;
		SizeT _length;
	};
#pragma pack(pop)	

	typedef range<const std::uint8_t, std::size_t> const_byte_range;
	typedef range<std::uint8_t, std::size_t> byte_range;



	template <typename T, typename SizeT>
	template <typename U>
	inline range<T, SizeT>::range(const range<U, SizeT> &u)
		: _data(u.begin()), _length(u.length())
	{	}

	template <typename T, typename SizeT>
	inline range<T, SizeT>::range(T *start, SizeT length)
		: _data(start), _length(static_cast<SizeT>(length))
	{	}

	template <typename T, typename SizeT>
	inline T *range<T, SizeT>::begin() const
	{	return _data;	}

	template <typename T, typename SizeT>
	inline T *range<T, SizeT>::end() const
	{	return _data + _length;	}

	template <typename T, typename SizeT>
	inline T *range<T, SizeT>::data() const
	{	return _data;	}

	template <typename T, typename SizeT>
	inline SizeT range<T, SizeT>::length() const
	{	return _length;	}

	template <typename T, typename SizeT>
	inline bool range<T, SizeT>::inside(const T *ptr) const
	{	return (begin() <= ptr) & (ptr < end());	}

	template <typename T, typename SizeT>
	inline range<T, SizeT> range<T, SizeT>::prefix(SizeT length_) const
	{	return range(_data, length_);	}

	template <typename T, typename SizeT>
	inline range<T, SizeT> range<T, SizeT>::suffix(SizeT start) const
	{	return range(_data + start, _length - start);	}


	template <typename T>
	inline range<const T, std::size_t> make_range(const std::vector<T> &from)
	{	return range<const T, std::size_t>(from.data(), from.size());	}

	template <typename T>
	inline range<T, std::size_t> make_range(std::vector<T> &from)
	{	return range<T, std::size_t>(from.data(), from.size());	}
}
