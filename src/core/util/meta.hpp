/**
 * @file meta.hpp
 *
 * @brief
 *     Some meta-programming utilities.
 *
 */

#pragma once

#include <type_traits>

#define MINIJAVA_INCLUDED_FROM_UTIL_META_HPP
#include "util/meta_preamble.tpp"
#undef MINIJAVA_INCLUDED_FROM_UTIL_META_HPP


namespace minijava
{

	/**
	 * @brief
	 *     Meta-programming meta-functions.
	 *
	 * These live in their own `namespace` because they use very innocent names
	 * like `all` and, being `template`s match about anything.
     *
	 */
	namespace meta
	{

		/**
		 * @brief
		 *     An empty `struct` that can be used for type singletons.
		 *
		 * @tparam T
		 *     types to store
		 *
		 */
		template <typename T>
		struct type_t {};

		/**
		 * @brief
		 *     An empty `struct` that can be used for type sequences.
		 *
		 * @tparam Ts
		 *     types to store in the list
		 *
		 */
		template <typename... Ts>
		struct types_t {};

		/**
		 * @brief
		 *     A replacement for the C++17 `std::conjunction` brought to C++14.
		 *
		 * If `bool(T::value)` is `true` for all `T` in `Ts...`, then
		 * `conjunction<Ts...>` is derived from `std::true_type`.  Otherwise it
		 * is derived from the first `T` in `Ts...` for which `bool(T)` is
		 * `false`.
		 *
		 * There is no parameter checking going on with this `template`.  If
		 * the contract is violated, anything may happen.
		 *
		 * @tparam Ts
		 *     types with a `static constexpr` member `value` that is
		 *     contextually convertible to `bool`
		 *
		 * @see
		 *     http://en.cppreference.com/w/cpp/types/conjunction
		 *
		 */
		template <typename... Ts>
	    using conjunction = detail::conjunction<Ts...>;

		/**
		 * @brief
		 *     A replacement for the C++17 `std::disjunction` brought to C++14.
		 *
		 * If `bool(T::value)` is `false` for all `T` in `Ts...`, then
		 * `disjunction<Ts...>` is derived from `std::false_type`.  Otherwise
		 * it is derived from the first `T` in `Ts...` for which `bool(T)` is
		 * `true`.
		 *
		 * There is no parameter checking going on with this `template`.  If
		 * the contract is violated, anything may happen.
		 *
		 * @tparam Ts
		 *     types with a `static constexpr` member `value` that is
		 *     contextually convertible to `bool`
		 *
		 * @see
		 *     http://en.cppreference.com/w/cpp/types/disjunction
		 *
		 */
		template <typename... Ts>
	    using disjunction = detail::disjunction<Ts...>;

		/**
		 * @brief
		 *     A replacement for the C++17 `std::negate` brought to C++14.
		 *
		 * `negate<T>` is derived from `std::true_type` is `bool(T::value)` is
		 * `false` and from `std::false_type` otherwise.
		 *
		 * There is no parameter checking going on with this `template`.  If
		 * the contract is violated, anything may happen.
		 *
		 * @tparam T
		 *     type with a `static constexpr` member `value` that is
		 *     contextually convertible to `bool`
		 *
		 * @see
		 *     http://en.cppreference.com/w/cpp/types/negate
		 *
		 */
		template <typename T>
		using negate = detail::negate<T>;

		/**
		 * @brief
		 *     A high-level wrapper around `conjunction` that supports
		 *     heterogenous programming.
		 *
		 * @tparam PredT
		 *     predicate meta-function that can be instantiated with all types
		 *     `Ts...` and provides a `static` `constexpr` member `value` that
		 *     is contextually convertible to `bool`
		 *
		 * There is no parameter checking going on with this `template`.  If
		 * the contract is violated, anything may happen.
		 *
		 * @tparam TypesT
		 *     type used to represent lists of types
		 *
		 * @tparam Ts
		 *     types to check the predicate for
		 *
		 * @param types
		 *     ignored (only used to deduce `Ts...`)
		 *
		 * @returns
		 *     whether `%PredT<T>::%value` is `true` for each `T` in `Ts...`
		 *
		 */
		template
		<
			template <typename> class PredT,
			template <typename...> class TypesT,
			typename... Ts
		>
		constexpr bool all(TypesT<Ts...> types) noexcept;

		/**
		 * @brief
		 *     A high-level wrapper around `disjunction` that supports
		 *     heterogenous programming.
		 *
		 * @tparam PredT
		 *     predicate meta-function that can be instantiated with all types
		 *     `Ts...` and provides a `static` `constexpr` member `value` that
		 *     is contextually convertible to `bool`
		 *
		 * There is no parameter checking going on with this `template`.  If
		 * the contract is violated, anything may happen.
		 *
		 * @tparam TypesT
		 *     type used to represent lists of types
		 *
		 * @tparam Ts
		 *     types to check the predicate for
		 *
		 * @param types
		 *     ignored (only used to deduce `Ts...`)
		 *
		 * @returns
		 *     whether `%PredT<T>::%value` is `true` for any `T` in `Ts...`
		 *
		 */
		template
		<
			template <typename> class PredT,
			template <typename...> class TypesT,
			typename... Ts
		>
		constexpr bool any(TypesT<Ts...> types) noexcept;

		/**
		 * @brief
		 *     A high-level wrapper around a negated `conjunction` that
		 *     supports heterogenous programming.
		 *
		 * @tparam PredT
		 *     predicate meta-function that can be instantiated with all types
		 *     `Ts...` and provides a `static` `constexpr` member `value` that
		 *     is contextually convertible to `bool`
		 *
		 * There is no parameter checking going on with this `template`.  If
		 * the contract is violated, anything may happen.
		 *
		 * @tparam TypesT
		 *     type used to represent lists of types
		 *
		 * @tparam Ts
		 *     types to check the predicate for
		 *
		 * @param types
		 *     ignored (only used to deduce `Ts...`)
		 *
		 * @returns
		 *     whether `%PredT<T>::%value` is `false` for each `T` in `Ts...`
		 *
		 */
		template
		<
			template <typename> class PredT,
			template <typename...> class TypesT,
			typename... Ts
		>
		constexpr bool none(TypesT<Ts...> types) noexcept;

	}  // namespace meta

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_UTIL_META_HPP
#include "util/meta_postamble.tpp"
#undef MINIJAVA_INCLUDED_FROM_UTIL_META_HPP
