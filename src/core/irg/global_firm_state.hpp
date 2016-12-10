/**
 * @file global_firm_state.hpp
 *
 * @brief
 *     RAII guard for `libfirm`'s global state sadness.
 *
 */

#pragma once


#include <memory>

namespace minijava
{
	namespace detail {
		struct global_firm_state_instance;
	}

	/**
	 * @brief
	 *    Holds the one global instance of libfirm.
	 *
	 */
	using global_firm_state = std::unique_ptr<detail::global_firm_state_instance, void(*)(detail::global_firm_state_instance*)>;

	/**
	 * @brief
	 *     Initializes libfirm.
	 *
	 * @throws std::logic_error
	 *     if libfirm was already initialized
	 *
	 */
	global_firm_state initialize_firm();

}  // namespace minijava
