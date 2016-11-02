/**
 * @file symbol_anchor.hpp
 *
 * @brief
 *     Supplementary data structures to tie `symbol`s to their pools in debug
 *     configuration.
 *
 * The stuff in this file should only ever be relevant for authors of symbol
 * pools.
 *
 */


#pragma once

#include <memory>


namespace minijava
{

	/**
	 * @brief
	 *     A monostate type used to assert the connection beween a symbol and
	 *     its pool via its address identity.
	 *
	 * Only symbol pool implementations should ever have to deal with this
	 * type.
	 *
	 */
	struct symbol_anchor
	{

		/**
		 * @brief
		 *     `return`s the singleton anchor of the empty `symbol`.
		 *
		 * In release configuration, this is a `nullptr`.  In debug
		 * configuration, the pointer references a unique object that will
		 * remain valid until static deinitialization occurs.
		 *
		 * In debug configuration, if an out-of-memory situation occurs while
		 * this function is called for the first time, `std::terminate()` will
		 * be called.
		 *
		 * @returns
		 *     singleton anchor of the empty `symbol`
		 *
		 */
		static const std::weak_ptr<symbol_anchor>& get_empty_symbol_anchor() noexcept;

		/**
		 * @brief
		 *     Creates a new `symbol_anchor`.
		 *
		 * In release configuration, this is a `nullptr`.  In debug
		 * configuration, it is a newly allocated smart pointer.  In release
		 * configuration, this function will never `throw` an exception.
		 *
		 * @returns
		 *     a new, unique symbol anchor
		 *
		 */
		static std::shared_ptr<symbol_anchor> make_symbol_anchor();

	};  // struct symbol_anchor

}  // namespace minijava
