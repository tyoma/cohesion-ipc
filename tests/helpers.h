#pragma once

#include <coipc/endpoint.h>
#include <coipc/misc.h>
#include <tuple>
#include <vector>

namespace coipc
{
	namespace tests
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

		guid_t generate_id();

		std::tuple<std::shared_ptr<FILE /*read end*/>, std::shared_ptr<FILE /*write end*/>> create_pipe();

		template <typename T, std::size_t size>
		inline std::vector<T> mkvector(T (&array_ptr)[size])
		{	return std::vector<T>(array_ptr, array_ptr + size);	}

		template <typename T, std::size_t size>
		inline range<T, std::size_t> mkrange(T (&array_ptr)[size])
		{	return range<T, std::size_t>(array_ptr, size);	}
	}
}
