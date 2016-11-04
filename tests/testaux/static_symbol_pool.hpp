/**
 * @file static_symbol_pool.hpp
 *
 * @brief
 *     A fake symbol pool that can normalize exactly one symbol.
 *
 */

#pragma once

#include "symbol_pool.hpp"

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
			: _anchor(get_satic_symbol_anchor())
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
		const std::shared_ptr<minijava::symbol_anchor>& get_satic_symbol_anchor()
		{
			static const auto static_symbol_anchor = std::make_shared<minijava::symbol_anchor>();
			return static_symbol_anchor;
		}


	private:
        minijava::symbol _symbol;
		entryptr_type _entry;
		std::shared_ptr<minijava::symbol_anchor> _anchor;
	};
}
