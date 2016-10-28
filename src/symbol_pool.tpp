#ifndef MINIJAVA_INCLUDED_FROM_SYMBOL_POOL_HPP
#error "Never `#include <symbol_pool.tpp>` directly; `#include <symbol_pool.hpp>` instead."
#endif

// TODO @Tobias Kahlert: Replace this stub implementation by an efficient one.


namespace minijava
{

	template<typename AllocT >
	symbol_pool<AllocT>::symbol_pool()
		: symbol_pool(AllocT())
	{
	}

	template<typename AllocT >
	symbol_pool<AllocT>::symbol_pool(const allocator_type& alloc)
		: _charAlloc(alloc)
		, _entryAlloc(alloc)
		, _anchor(std::make_shared<symbol_debug_pool_anchor>())
	{
	}

	template<typename AllocT >
	symbol_pool<AllocT>::symbol_pool(symbol_pool&& old)
		: _charAlloc(std::move(old._charAlloc))
		, _entryAlloc(std::move(old._entryAlloc))
		, _pool(std::move(old._pool))
	{
	}

	template<typename AllocT>
	symbol_pool<AllocT>::~symbol_pool()
	{
		assert(_anchor.unique());

		for(auto& entry: _pool)
		{
			char_allocator_traits::deallocate(_charAlloc, const_cast<char*>(entry->cstr), entry->size);
			entry_allocator_traits::deallocate(_entryAlloc, const_cast<entry_type*>(entry), 1);
		}
	}

	template<typename AllocT >
	symbol symbol_pool<AllocT>::normalize(const std::string& text)
	{
		std::hash<std::string> hasher;
		std::size_t sym_hash = hasher(text);

		const entry_type find_entry(text.c_str(), text.size(), sym_hash);

		auto entry_it = _pool.find(&find_entry);

		if(entry_it == _pool.end())
		{
			const char * sym_mem = create_string(text);
			const entry_type * insert_entry = create_entry(sym_mem, text.size(), sym_hash);

			std::tie(entry_it, std::ignore) = _pool.insert(insert_entry);
		}

		return symbol(*entry_it, _anchor);
	}

	template<typename AllocT >
	bool symbol_pool<AllocT>::contains(const std::string& text) const
	{
		const auto hash_fn = std::hash<std::string>();
		const auto hash = hash_fn(text);
		const entry_type entry(text.c_str(), text.size(), hash);
		return (_pool.find(&entry) != _pool.cend());
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
		return _charAlloc;
	}

	template<typename AllocT >
	const char * symbol_pool<AllocT>::create_string(const std::string& str)
	{
		char * str_mem = char_allocator_traits::allocate(_charAlloc, str.size() + 1);
		std::copy(std::begin(str), std::end(str), str_mem);
		str_mem[str.size()] = '\0';
		return str_mem;
	}

	template<typename AllocT >
	const typename symbol_pool<AllocT>::entry_type *
		symbol_pool<AllocT>::create_entry(const char * str_mem, std::size_t size, std::size_t hash)
	{
		entry_type * entry_mem = entry_allocator_traits::allocate(_entryAlloc, 1);
		entry_allocator_traits::construct(_entryAlloc, entry_mem, str_mem, size, hash);

		return entry_mem;
	}

}  // namespace minijava
