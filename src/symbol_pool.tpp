#ifndef MINIJAVA_INCLUDED_FROM_symbol_pool_HPP
#error "Never `#include <symbol_pool.tpp>` directly; `#include <symbol_pool.hpp>` instead."
#endif

// TODO @Tobias Kahlert: Replace this stub implementation by an efficient one.


namespace minijava
{

	template <typename InnerAllocT, typename OuterAllocT>
	symbol_pool<InnerAllocT, OuterAllocT>::symbol_pool()
	{
	}

	template <typename InnerAllocT, typename OuterAllocT>
	symbol_pool<InnerAllocT, OuterAllocT>::symbol_pool(
		const inner_allocator_type& inner,
		const outer_allocator_type& outer
	) : _pool{
		0,
		typename hash_set_type::hasher{},
		typename hash_set_type::key_equal{},
		scoped_allocator_type{outer, inner}
	}
	{
	}

	template <typename InnerAllocT, typename OuterAllocT>
	symbol symbol_pool<InnerAllocT, OuterAllocT>::normalize(const std::string& text)
	{
		auto pos = _pool.find(text);
		if (pos == _pool.cend())
			pos = _pool.insert(pos, text);
		return symbol::create_from_canonical_pointer(pos->c_str());
	}

	template <typename InnerAllocT, typename OuterAllocT>
	bool symbol_pool<InnerAllocT, OuterAllocT>::contains(const std::string& text) const
	{
		return (_pool.find(text) != _pool.cend());
	}

	template <typename InnerAllocT, typename OuterAllocT>
	std::size_t symbol_pool<InnerAllocT, OuterAllocT>::size() const noexcept
	{
		return _pool.size();
	}

	template <typename InnerAllocT, typename OuterAllocT>
	bool symbol_pool<InnerAllocT, OuterAllocT>::empty() const noexcept
	{
		return _pool.empty();
	}

	template <typename InnerAllocT, typename OuterAllocT>
	InnerAllocT symbol_pool<InnerAllocT, OuterAllocT>::get_inner_allocator() const
	{
		return _pool.get_allocator().inner_allocator();
	}

	template <typename InnerAllocT, typename OuterAllocT>
	OuterAllocT symbol_pool<InnerAllocT, OuterAllocT>::get_outer_allocator() const
	{
		return _pool.get_allocator().outer_allocator();
	}

}  // namespace minijava
