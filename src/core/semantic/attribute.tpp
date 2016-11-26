#ifndef MINIJAVA_INCLUDED_FROM_ATTRIBUTE_HPP
#error "Never `#include <semantic/attribute.tpp>` directly; `#include <semantic/attribute.hpp>` instead."
#endif

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <utility>


namespace minijava
{

	template <typename... NodeTs>
	bool ast_node_filter<NodeTs...>::dynamic_check(const ast::node& node) const noexcept
	{
		if (node.id() == 0) {
			return false;
		}
		const std::initializer_list<const void*> pointers = {
			dynamic_cast<const NodeTs*>(&node)...
		};
		return std::any_of(
			std::begin(pointers), std::end(pointers),
			[](auto p){ return p != nullptr; }
		);
	}


	template <typename T, typename NodeFilterT, typename AllocT>
	ast_attributes<T, NodeFilterT, AllocT>::ast_attributes() noexcept
	{
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	ast_attributes<T, NodeFilterT, AllocT>::ast_attributes(const NodeFilterT& filter) noexcept
		: ast_attributes{filter, allocator_type{}}
	{
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	ast_attributes<T, NodeFilterT, AllocT>::ast_attributes(const allocator_type& alloc) noexcept
		: ast_attributes{NodeFilterT{}, alloc}
	{
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	ast_attributes<T, NodeFilterT, AllocT>::ast_attributes(const NodeFilterT& filter, const allocator_type& alloc) noexcept
		: NodeFilterT{filter}, _data{alloc}
	{
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	bool
	ast_attributes<T, NodeFilterT, AllocT>::empty() const noexcept
	{
		return _data.empty();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::size_type
	ast_attributes<T, NodeFilterT, AllocT>::size() const noexcept
	{
		return _data.size();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::size_type
	ast_attributes<T, NodeFilterT, AllocT>::max_size() const noexcept
	{
		return _data.max_size();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::iterator
	ast_attributes<T, NodeFilterT, AllocT>::begin() noexcept
	{
		return _data.begin();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::iterator
	ast_attributes<T, NodeFilterT, AllocT>::end() noexcept
	{
		return _data.end();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::const_iterator
	ast_attributes<T, NodeFilterT, AllocT>::begin() const noexcept
	{
		return _data.begin();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::const_iterator
	ast_attributes<T, NodeFilterT, AllocT>::end() const noexcept
	{
		return _data.end();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::const_iterator
	ast_attributes<T, NodeFilterT, AllocT>::cbegin() const noexcept
	{
		return _data.cbegin();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::const_iterator
	ast_attributes<T, NodeFilterT, AllocT>::cend() const noexcept
	{
		return _data.cend();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::iterator
	ast_attributes<T, NodeFilterT, AllocT>::insert(const const_iterator hint, const value_type& value)
	{
		assert(get_filter().dynamic_check(*value.first));
		const auto res = _data.insert(hint, value);
		assert(key_eq()(res->first, value.first));
		return res;
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::iterator
	ast_attributes<T, NodeFilterT, AllocT>::insert(const const_iterator hint, value_type&& value)
	{
		const auto ptr = value.first;
		assert(get_filter().dynamic_check(*ptr));
		const auto res = _data.insert(hint, std::move(value));
		assert(key_eq()(res->first, ptr));
		(void) ptr;  // Avoid -Wunused-variable if assertions are disabled
		return res;
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	std::pair<typename ast_attributes<T, NodeFilterT, AllocT>::iterator, bool>
	ast_attributes<T, NodeFilterT, AllocT>::insert(const value_type& value)
	{
		assert(get_filter().dynamic_check(*value.first));
		const auto res = _data.insert(value);
		assert(!res.second || key_eq()(res.first->first, value.first));
		return res;
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	std::pair<typename ast_attributes<T, NodeFilterT, AllocT>::iterator, bool>
	ast_attributes<T, NodeFilterT, AllocT>::insert(value_type&& value)
	{
		const auto ptr = value.first;
		assert(get_filter().dynamic_check(*ptr));
		const auto res = _data.insert(std::move(value));
		assert(!res.second || key_eq()(res.first->first, ptr));
		(void) ptr;  // Avoid -Wunused-variable if assertions are disabled
		return res;
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::iterator
	ast_attributes<T, NodeFilterT, AllocT>::find(const key_type& k)
	{
		assert(get_filter().dynamic_check(*k));
		const auto pos = _data.find(k);
		assert((pos == _data.end()) || key_eq()(pos->first, k));
		return pos;
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::const_iterator
	ast_attributes<T, NodeFilterT, AllocT>::find(const key_type& k) const
	{
		assert(get_filter().dynamic_check(*k));
		const auto pos = _data.find(k);
		assert((pos == _data.end()) || key_eq()(pos->first, k));
		return pos;
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::size_type
	ast_attributes<T, NodeFilterT, AllocT>::count(const key_type& k) const
	{
		assert(get_filter().dynamic_check(*k));
		return _data.count(k);
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::node_filter_type
	ast_attributes<T, NodeFilterT, AllocT>::get_filter() const noexcept
	{
		return *this;
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::allocator_type
	ast_attributes<T, NodeFilterT, AllocT>::get_allocator() const noexcept
	{
		return _data.get_allocator();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::hasher
	ast_attributes<T, NodeFilterT, AllocT>::hash_function() const noexcept
	{
		return _data.hash_function();
	}

	template <typename T, typename NodeFilterT, typename AllocT>
	typename ast_attributes<T, NodeFilterT, AllocT>::key_equal
	ast_attributes<T, NodeFilterT, AllocT>::key_eq() const noexcept
	{
		return _data.key_eq();
	}


}  // namespace minijava
