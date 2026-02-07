#pragma once

namespace coipc
{
	template <typename F>
	struct arguments_pack;

	template <typename R>
	struct arguments_pack<R ()>
	{
		enum {	count = 0,	};
		typedef R return_type;
	};

	template <typename R, typename T1>
	struct arguments_pack<R (T1)>
	{
		enum {	count = 1,	};
		typedef R return_type;
		typedef T1 argument_1_type;
	};

	template <typename R, typename T1, typename T2>
	struct arguments_pack<R (T1, T2)>
	{
		enum {	count = 2,	};
		typedef R return_type;
		typedef T1 argument_1_type;
		typedef T2 argument_2_type;
	};

	template<typename F, typename R>
	arguments_pack<R ()> argtype_helper(R (F::*)());

	template<typename F, typename R>
	arguments_pack<R ()> argtype_helper(R (F::*)() const);

	template<typename F, typename R, typename T1>
	arguments_pack<R (T1)> argtype_helper(R (F::*)(T1));

	template<typename F, typename R, typename T1>
	arguments_pack<R (T1)> argtype_helper(R (F::*)(T1) const);

	template<typename F, typename R, typename T1, typename T2>
	arguments_pack<R (T1, T2)> argtype_helper(R (F::*)(T1, T2));

	template<typename F, typename R, typename T1, typename T2>
	arguments_pack<R (T1, T2)> argtype_helper(R (F::*)(T1, T2) const);

	template <typename F>
	struct argument_traits
	{
		typedef decltype(argtype_helper(&F::operator())) types;
	};
}
