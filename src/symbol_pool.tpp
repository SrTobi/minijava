#ifndef MINIJAVA_INCLUDED_FROM_SYMBOL_POOL_HPP
#error "Never `#include <symbol_pool.tpp>` directly; `#include <symbol_pool.hpp>` instead."
#endif

// TODO @Tobias Kahlert: Replace this stub implementation by an efficient one.


namespace minijava
{
	template<typename AllocT>
	struct symbol_pool<AllocT>::symbol_entry_string_cmp
	{
		bool operator()(const std::string& str, const entryptr_type& entry) const
		{
			return entry->size == str.size() && str == entry->cstr;
		}
	};

	template<typename AllocT >
	symbol_pool<AllocT>::symbol_pool()
		: symbol_pool(AllocT())
	{
	}

	template<typename AllocT >
	symbol_pool<AllocT>::symbol_pool(const allocator_type& alloc)
		: _alloc(alloc)
		, _anchor(std::make_shared<symbol_debug_pool_anchor>())
	{
	}

	template<typename AllocT >
	symbol_pool<AllocT>::symbol_pool(symbol_pool&& old)
		: _alloc(std::move(old._alloc))
		, _pool(std::move(old._pool))
		, _anchor(std::move(old._anchor))
	{
		old._anchor = std::make_shared<symbol_debug_pool_anchor>();
	}

	template<typename AllocT>
	symbol_pool<AllocT>::~symbol_pool()
	{
		_invalidate_pool();
	}

	template<typename AllocT>
	symbol_pool<AllocT>& symbol_pool<AllocT>::operator=(symbol_pool&& old)
	{
		_invalidate_pool();
		_alloc = std::move(old._alloc);
		_pool = std::move(old._pool);
		_anchor = std::move(old._anchor);
		old._anchor = std::make_shared<symbol_debug_pool_anchor>();

		return *this;
	}

	template<typename AllocT >
	symbol symbol_pool<AllocT>::normalize(const std::string& text)
	{
		std::hash<std::string> hash_fn;
		auto entry_it = _pool.find(text, hash_fn, symbol_entry_string_cmp());

		if(entry_it == _pool.end())
		{
			auto insert_entry = symbol_entry::allocate(_alloc, text);

			std::tie(entry_it, std::ignore) = _pool.insert(std::move(insert_entry));
		}

		return symbol(entry_it->get(), _anchor);
	}

	template<typename AllocT >
	bool symbol_pool<AllocT>::contains(const std::string& text) const
	{
		const auto hash_fn = std::hash<std::string>();
		return (_pool.find(text, hash_fn, symbol_entry_string_cmp()) != _pool.cend());
	}

	template<typename AllocT >
	std::size_t symbol_pool<AllocT>::size() const noexcept
	{
		return _pool.size();
	}

	template<typename AllocT >
	bool symbol_pool<AllocT>::empty() const noexcept
	{
		return _pool.empty();
	}

	template<typename AllocT >
	AllocT symbol_pool<AllocT>::get_allocator() const
	{
		return _alloc;
	}

	template<typename AllocT >
	void symbol_pool<AllocT>::_invalidate_pool()
	{
		_anchor->pool_available = false;
	}


}  // namespace minijava
