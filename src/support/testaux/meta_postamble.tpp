#ifndef TESTAUX_INCLUDED_FROM_META_HPP
#error "Never `#include <testaux/meta_postamble.tpp>` directly; `#include <testaux/meta.hpp>` instead."
#endif


namespace testaux
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

}  // namespace testaux
