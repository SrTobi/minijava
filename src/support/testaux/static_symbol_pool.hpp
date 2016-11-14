/**
 * @file static_symbol_pool.hpp
 *
 * @brief
 *     A fake symbol pool that can normalize exactly one symbol.
 *
 */

#pragma once

#include "symbol/symbol_pool.hpp"

namespace testaux
{
	/**
	 * @brief
	 *     Used to allocate one symbol.
	 *
	 * A `static_symbol_pool` can be used to create a single `symbol`.  Its
	 * special property however is that its created symbols are allowed to be
	 * compared with one another.  The programmer has to take care that only
	 * one static_symbol_pool is created per possible string value! Two symbols
	 * with the same content from different `static_symbol_pools` will *not*
	 * compare equal!  The only exception to this rule is the empty symbol.
	 *
	 */
	class static_symbol_pool
	{
	private:

		/** @brief Type alias for a smart pointer holding a `symbol_entry`. */
		using entryptr_type = minijava::unique_symbol_entr_ptr<>;

	public:
		/**
		 * @brief
		 *     Constructs the static_symbol_pool with a given string
		 *
		 * @param str
		 *     The content for the symbol that is created by this pool.
		 */
		static_symbol_pool(const std::string& str)
			: _anchor(get_static_symbol_anchor())
		{
            if (!str.empty()) {
				auto alloc = std::allocator<minijava::symbol_entry>{};
				const auto hash = std::hash<std::string>{}(str);
                _entry = minijava::new_symbol_entry(alloc, hash, str.size(), str.data());
                _symbol = minijava::symbol(_entry.get(), _anchor);
            }
		}

		/**
		 * @brief
		 *     Destructs the static_symbol_pool.
		 */
		~static_symbol_pool()
		{
		}

		/**
		 * @brief
		 *     Returns the pool's symbol
		 *
		 * @returns
		 *     the one and only symbol of this pool.
		 */
		minijava::symbol get() const
		{
			return _symbol;
		}

	private:
		/**
		 * @brief
		 *     creates a symbol anchor for this pool
		 *
		 * @return
		 *     symbol anchor
		 */
		const std::shared_ptr<minijava::symbol_anchor>& get_static_symbol_anchor()
		{
			static const auto static_symbol_anchor = std::make_shared<minijava::symbol_anchor>();
			return static_symbol_anchor;
		}


	private:
		/** @brief Single symbol contained in this pool */
        minijava::symbol _symbol;
		/** @brief Entry for the single symbol contained in this pool */
		entryptr_type _entry;
		/** @brief Symbol anchor for this pool */
		std::shared_ptr<minijava::symbol_anchor> _anchor;
	};
}
