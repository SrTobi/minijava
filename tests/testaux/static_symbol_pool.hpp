#pragma once

#include "symbol_pool.hpp"

namespace testaux
{
	/**
	 * @brief
	 *     Used to allocate one symbol.
	 *
	 * The static_symbol_pool can be used to create a single symbol.
	 * Its special property however is, that its created symbols,
	 * are allowed to be compared with one another.
	 * The programmer has to take care that only one static_symbol_pool
	 * is created per possible string value! Two symbols with the same
	 * content from different static_symbol_pools will NOT compare equal!
	 * The only exception to this rule is the empty symbol.
	 */
	class static_symbol_pool: private boost::noncopyable
	{
	private:
		using entryptr_type = minijava::symbol_entry::ptr<std::allocator<char>>;
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
            if(!str.empty()){
                _entry = minijava::symbol_entry::allocate(_allocator, str);
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
		const std::shared_ptr<minijava::symbol_debug_pool_anchor>& get_satic_symbol_anchor()
		{
			static const auto static_symbol_anchor = std::make_shared<minijava::symbol_debug_pool_anchor>();
			return static_symbol_anchor;
		}


	private:
        minijava::symbol _symbol;
		std::allocator<char> _allocator;
		entryptr_type _entry;
		std::shared_ptr<minijava::symbol_debug_pool_anchor> _anchor;
	};
}
