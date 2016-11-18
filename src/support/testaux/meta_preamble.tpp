#ifndef TESTAUX_INCLUDED_FROM_META_HPP
#error "Never `#include <testaux/meta_preamble.tpp>` directly; `#include <testaux/meta.hpp>` instead."
#endif


namespace testaux
{

	namespace meta
	{

		namespace detail
		{

			// conjunction

			template <typename...>
			struct conjunction;

			template<>
			struct conjunction<> : std::true_type {};

			template<typename HeadT, typename... TailTs>
			struct conjunction<HeadT, TailTs...> : std::conditional_t
			<
				bool(HeadT::value),
				conjunction<TailTs...>,
				HeadT
			> {};

			// disjunction

			template <typename...>
			struct disjunction;

			template<>
			struct disjunction<> : std::false_type {};

			template<typename HeadT, typename... TailTs>
			struct disjunction<HeadT, TailTs...> : std::conditional_t
			<
				not bool(HeadT::value),
				disjunction<TailTs...>,
				HeadT
			> {};

			// negate

			template <typename T>
			struct negate : std::integral_constant<bool, not bool(T::value)> {};

		}  // namespace detail

	}  // namespace meta

}  // namespace testaux
