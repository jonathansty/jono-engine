#pragma once

namespace rtti
{
	class Object;

	template<typename T>
	struct function_traits : function_traits< decltype(&T::operator()) > {};

	template<typename R, typename C, typename...Args>
	struct function_traits<R(C::*)(Args...)> {
		static constexpr size_t arg_count = sizeof...(Args);

		using class_type = C;
		using return_type = R;
		using arg_types = std::tuple<Args...>;
	};

	template <class T, T... I>
	struct integer_sequence
	{
		template <T N> using append = integer_sequence<T, I..., N>;
		static std::size_t size() { return sizeof...(I); }
		using next = append<sizeof...(I)>;
		using type = T;
	};

	template <std::size_t... I>
	using index_sequence = integer_sequence<std::size_t, I...>;

	template <class T, T Index, std::size_t N>
	struct sequence_generator
	{
		using type = typename sequence_generator<T, Index - 1, N - 1>::type::next;
	};

	template <class T, T Index>
	struct sequence_generator<T, Index, 0ul> { using type = integer_sequence<T>; };

	template <std::size_t... I>
	using index_sequence = integer_sequence<std::size_t, I...>;

	template <class T, T N>
	using make_integer_sequence = typename sequence_generator<T, N, N>::type;

	template <std::size_t N>
	using make_index_sequence = make_integer_sequence<std::size_t, N>;


	template<typename F, size_t Index>
	struct param_types
	{
		using type = typename std::tuple_element<Index, typename function_traits<F>::arg_types>::type;
	};

	namespace MethodTypes
	{
		struct VoidMemberType
		{
			using type = VoidMemberType;
		};

		struct ReturnMemberType
		{
			using type = ReturnMemberType;
		};

		template< bool B, class T, class F >
		using conditional_t = typename std::conditional<B, T, F>::type;

		template<typename F>
		struct is_void_func : conditional_t< std::is_same<typename function_traits<F>::return_type, void>::value,
			std::true_type,
			std::false_type
		>
		{
		};

		template<typename T>
		struct MethodType : conditional_t< is_void_func<T>::value, VoidMemberType, ReturnMemberType> {};
	}

	template<typename F, size_t Index>
	using param_types_t = typename param_types<F, Index>::type;

	template<typename F, typename MethodType, typename IndexSequence>
	struct MethodInvoker;

	bool check_all_true(bool arg1);

	template<typename... BoolArgs>
	bool check_all_true(bool arg1, BoolArgs... args) { return arg1 & check_all_true(args...); }

	template<typename F, std::size_t...ArgCount>
	struct MethodInvoker<F, MethodTypes::ReturnMemberType, index_sequence<ArgCount...> >
	{
		template<typename... TArgs>
		static rtti::Object invoke(const F& func_ptr, const rtti::Object& obj, const TArgs&...args)
		{
			using class_t = typename function_traits<F>::class_type;
			class_t* ptr = obj.get<class_t>();
			bool all_params = check_all_true(args.template is_type< param_types_t<F, ArgCount>>()...);

			if (ptr && all_params)
			{
				return rtti::Object::create_with_copy((ptr->*func_ptr)(args. template get_value< param_types_t<F, ArgCount>>()...));
			}
			return {};
		}
	};

	template<typename F, std::size_t...ArgCount>
	struct MethodInvoker<F, MethodTypes::VoidMemberType, index_sequence<ArgCount...> >
	{
		template<typename... TArgs>
		static rtti::Object invoke(const F& func_ptr, const rtti::Object& obj, const TArgs&...args)
		{
			using class_t = typename function_traits<F>::class_type;
			class_t* ptr = obj.get<class_t>();
			bool all_params = check_all_true(args.template is_type< param_types_t<F, ArgCount>>()...);

			if (ptr && all_params)
			{
				(ptr->*func_ptr)(args. template get_value< param_types_t<F, ArgCount>>()...);
				return rtti::Object{};
			}
			return {};
		}
	};


	class MethodBase
	{
	public:
		virtual ~MethodBase() {}

		using Argument = rtti::Object;

		virtual rtti::Object invoke(rtti::Object& obj) = 0;
		virtual rtti::Object invoke(rtti::Object& obj, Argument const& arg0) = 0;
		virtual rtti::Object invoke(rtti::Object& obj, Argument const& arg0, Argument const& arg1) = 0;
	};

	template<typename F>
	class Method : public MethodBase
	{
	public:
		using Argument = MethodBase::Argument;

		using ClassType = typename function_traits<F>::class_type;
		using ReturnType = typename function_traits<F>::return_type;
		using Arguments = typename function_traits<F>::arg_types;
		using MethodType = typename MethodTypes::MethodType<F>::type;

		using arg_index_sequence = make_index_sequence<function_traits<F>::arg_count>;
		using invoker_class = MethodInvoker<F, MethodType, arg_index_sequence>;

		Method(F func) : MethodBase(), _func(func)
		{

		}

		rtti::Object invoke(rtti::Object& obj)  override
		{
			if constexpr (function_traits<F>::arg_count == 0)
			{
				return invoker_class::invoke(_func, obj);
			}
			return rtti::Object();
		}

		rtti::Object invoke(rtti::Object& obj, Argument const& arg0) override
		{
			if constexpr (function_traits<F>::arg_count == 1)
			{
				return invoker_class::invoke(_func, obj, arg0);
			}
			return {};
		}

		rtti::Object invoke(rtti::Object& obj, Argument const& arg0, Argument const& arg1) override
		{
			if constexpr (function_traits<F>::arg_count == 2)
			{
				return invoker_class::invoke(_func, obj, arg0, arg1);
			}
			return {};
		}

	private:
		F _func;
	};

}


