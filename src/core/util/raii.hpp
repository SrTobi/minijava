/**
 * @file raii.hpp
 *
 * @brief
 *     Useful convenience features for practicing RAII.
 *
 */

#pragma once

#include <memory>
#include <type_traits>


namespace minijava
{

	/**
	 * @brief
	 *     Callable that sets an address to a constant value.
	 *
	 * `T` must be `noexcept` copyable.
	 *
	 * @tparam T
	 *     type of the value to set
	 *
	 */
	template <typename T>
	class const_setter
	{

		static_assert(std::is_nothrow_copy_constructible<T>{}, "");
		static_assert(std::is_nothrow_copy_assignable<T>{}, "");

	public:

		/**
		 * @brief
		 *     Creates a new setter that will set to the given value.
		 *
		 * @param value
		 *     value this setter will set
		 *
		 */
		const_setter(const T value) noexcept : _value{value}
		{
		}

		/**
		 * @brief
		 *     Sets the object referenced by the given pointer.
		 *
		 * If the pointer does not reference an assignable object, the behavior
		 * is undefined.  This function is declared `noexcept` nonetheless
		 * because its purpose it to be used inside destructors.
		 *
		 * @param dest
		 *     pointer to the object to set
		 *
		 */
		void operator()(T* dest) const noexcept
		{
			*dest = _value;
		}

	private:

		/** @brief Value to set to. */
		T _value{};

	};  // class const_setter


	/**
	 * @brief
	 *     Sets `target` to `value` and `return`s an RAII guard that will reset
	 *     it in its destructor.
	 *
	 * @tparam T
	 *     type of the value (must be `noexcept` copyable)
	 *
	 * @param target
	 *     destination to set and reset
	 *
	 * @param value
	 *     value to set temporarily
	 *
	 * @returns
	 *     RAII guard of unspecified type
	 *
	 */
	template <typename T>
	auto set_temporarily(T& target, const T value)
		noexcept(
			std::enable_if_t<
				noexcept(target = value) && noexcept(T{value}),
				std::true_type
			>{}
		)
	{
		auto del = const_setter<T>{target};
		auto guard = std::unique_ptr<T, const_setter<T>>{&target, del};
		target = value;
		return guard;
	}

	/**
	 * @brief
	 *     Increments `target` by `amount` and `return`s an RAII guard that
	 *     will reset it in its destructor.
	 *
	 * @tparam T
	 *     arithmetic type of the value (must be `noexcept` copyable)
	 *
	 * @param target
	 *     destination to set and reset
	 *
	 * @param amount
	 *     amount by which to increment temporarily
	 *
	 * @returns
	 *     RAII guard of unspecified type
	 *
	 */
	template <typename T>
	auto increment_temporarily(T& target, const T amount = T{1})
		noexcept(std::enable_if_t<std::is_arithmetic<T>{}, std::true_type>{})
	{
		auto del = const_setter<T>{target};
		auto guard = std::unique_ptr<T, const_setter<T>>{&target, del};
		target += amount;
		return guard;
	}

}  // namespace minijava
