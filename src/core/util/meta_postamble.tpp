#ifndef MINIJAVA_INCLUDED_FROM_UTIL_META_HPP
#error "Never `#include <util/meta_postamble.tpp>` directly; `#include <util/meta.hpp>` instead."
#endif


namespace minijava
{

	namespace meta
	{

		template
		<
			template <typename> class PredT,
			template <typename...> class TypesT,
			typename... Ts
		>
		constexpr bool all(TypesT<Ts...>) noexcept
		{
			return conjunction<PredT<Ts>...>{};
		}


		template
		<
			template <typename> class PredT,
			template <typename...> class TypesT,
			typename... Ts
		>
		constexpr bool any(TypesT<Ts...>) noexcept
		{
			return disjunction<PredT<Ts>...>{};
		}


		template
		<
			template <typename> class PredT,
			template <typename...> class TypesT,
			typename... Ts
		>
		constexpr bool none(TypesT<Ts...>) noexcept
		{
			return conjunction<negate<PredT<Ts>>...>{};
		}

	}  // namespace meta

}  // namespace minijava
