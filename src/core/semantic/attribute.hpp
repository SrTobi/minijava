/**
 * @file attribute.hpp
 *
 * @brief
 *     Attributes for AST nodes.
 *
 */

#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>

#include "exceptions.hpp"
#include "parser/ast.hpp"

namespace minijava
{

	namespace meta
	{

		template <typename T>
		struct type {};

		template <typename... Ts>
		struct types {};

	}

	struct ast_node_filter
	{

		template<template <typename> class TypeT, typename T>
		static constexpr bool static_check(TypeT<T>) noexcept
		{
			return std::is_base_of<ast::node, T>{};
		}

		bool dynamic_check(const ast::node& node) const noexcept
		{
			return (node.id() != 0);
		}

	};

	struct ast_node_ptr_hash
	{
		std::size_t operator()(const ast::node* nodeptr) const
		{
			assert((nodeptr != nullptr) && (nodeptr->id() != 0));
			return nodeptr->id();
		}
	};

	/**
	 *
	 * The policy types `NodeFilterT` and `AllocT` shall not `throw` exceptions
	 * in their copy constructors and assignment operators and (if they are
	 * default-constructible) their default constructors.  This is not enforced
	 * via `static_assert`ions, though, because doing so would perclude a lot
	 * of potentially useful policies (in particular, lambdas) which are known
	 * to never `throw` but just are not formally declared `noexcept`.  If this
	 * constraint is violated and any policy object does `throw`, the behavior
	 * is undefined.
	 *
	 */
	template
	<
		typename T,
		typename NodeFilterT = ast_node_filter,
		typename AllocT = std::allocator<std::pair<const ast::node *const, T>>
	>
	class ast_attributes : private NodeFilterT
	{

		static_assert(
			std::is_nothrow_copy_constructible<NodeFilterT>{},
			"The node filter policy must be copyable and this operation shall not throw"
		);

		static_assert(
			std::is_nothrow_copy_constructible<AllocT>{},
			"The allocator must be copyable and this operation shall not throw"
		);

		template <typename NodeT, typename ResultT = void>
		using apply_static_node_filter_t = std::enable_if_t<
			NodeFilterT::static_check(meta::type<std::decay_t<NodeT>>{}),
			ResultT
		>;

	public:

		using key_type = const ast::node*;
		using mapped_type = T;
		using value_type = std::pair<const key_type, mapped_type>;
		using hasher = ast_node_ptr_hash;
		using key_equal = std::equal_to<key_type>;
		using allocator_type = AllocT;
		using pointer = typename std::allocator_traits<allocator_type>::pointer;
		using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using iterator = typename std::unordered_map<key_type, mapped_type, hasher, key_equal, allocator_type>::iterator;
		using const_iterator = typename std::unordered_map<key_type, mapped_type, hasher, key_equal, allocator_type>::const_iterator;
		using node_filter_type = NodeFilterT;

		/**
		 * @brief
		 *     Creates an empty map.
		 *
		 * This constructor is only available if `NodeFilterT` and `AllocT` are
		 * both default-constructible.  These default constructors shall not
		 * `throw` exceptions; otherwise the behavior is undefined.
		 *
		 */
		ast_attributes() noexcept;

		/**
		 * @brief
		 *     Creates an empty map with the given filter policy and a
		 *     default-constructed allocator.
		 *
		 * This constructor is only available if `AllocT` is
		 * default-constructible.  This default constructor shall not `throw`
		 * exceptions; otherwise the behavior is undefined.
		 *
		 * @param filter
		 *     filter policy object
		 *
		 */
		ast_attributes(const node_filter_type& filter) noexcept;

		/**
		 * @brief
		 *     Creates an empty map with the given allocator and a
		 *     default-constructed filter policy.
		 *
		 * This constructor is only available if `NodeFilterT` is
		 * default-constructible.  This default constructor shall not `throw`
		 * exceptions; otherwise the behavior is undefined.
		 *
		 * @param alloc
		 *     allocator object
		 *
		 */
		ast_attributes(const allocator_type& alloc) noexcept;

		/**
		 * @brief
		 *     Creates an empty map with the given filter policy and allocator.
		 *
		 * @param filter
		 *     filter policy object
		 *
		 * @param alloc
		 *     allocator object
		 *
		 */
		ast_attributes(const node_filter_type& filter, const allocator_type& alloc) noexcept;

		/**
		 * @brief
		 *     Tests whether the map is empty;
		 *
		 * @returns
		 *     whether the map is empty
		 *
		 */
		bool empty() const noexcept;

		/**
		 * @brief
		 *     `return`s the number of entries in the map;
		 *
		 * @returns
		 *     number of entries in the map
		 *
		 */
		size_type size() const noexcept;

		/**
		 * @brief
		 *     `return`s the maximum number of entries in the map;
		 *
		 * @returns
		 *     a rather meaningless huge integer value
		 *
		 */
		size_type max_size() const noexcept;

		/**
		 * @brief
		 *     `return`s an iterator pointing to the first element.
		 *
		 * @returns
		 *     iterator pointing to the first element
		 *
		 */
		iterator begin() noexcept;

		/**
		 * @brief
		 *     `return`s an iterator pointing after the last element.
		 *
		 * @returns
		 *     iterator pointing after the last element
		 *
		 */
		iterator end() noexcept;

		/**
		 * @brief
		 *     `return`s a constant iterator pointing to the first element.
		 *
		 * @returns
		 *     constant iterator pointing to the first element
		 *
		 */
		const_iterator begin() const noexcept;

		/**
		 * @brief
		 *     `return`s a constant iterator pointing after the last element.
		 *
		 * @returns
		 *     constant iterator pointing after the last element
		 *
		 */
		const_iterator end() const noexcept;

		/**
		 * @brief
		 *     `return`s a constant iterator pointing to the first element.
		 *
		 * @returns
		 *     constant iterator pointing to the first element
		 *
		 */
		const_iterator cbegin() const noexcept;

		/**
		 * @brief
		 *     `return`s a constant iterator pointing after the last element.
		 *
		 * @returns
		 *     constant iterator pointing after the last element
		 *
		 */
		const_iterator cend() const noexcept;

		/**
		 * @brief
		 *     Inserts a new entry into the map if it is not already present.
		 *
		 * If the map already contains an entry for a node with a different
		 * address but with the same ID, the behavior is undefined.
		 *
		 * The behavior is also undefined unless
		 *
		 *     get_filter().dynamic_check(*value.first)
		 *
		 * passes.
		 *
		 * Otherwise, this function has the same effect as
		 * `std::unordered_map::insert`.
		 *
		 * @param value
		 *     value to insert
		 *
		 * @param hint
		 *     non-binding suggestion where to insert the element
		 *
		 * @returns
		 *     iterator to the newly inserted item or the item that prevented
		 *     insertion
		 *
		 */
		iterator insert(const_iterator hint, const value_type& value);

		/**
		 * @brief
		 *     Inserts a new entry into the map if it is not already present.
		 *
		 * If the map already contains an entry for a node with a different
		 * address but with the same ID, the behavior is undefined.
		 *
		 * The behavior is also undefined unless
		 *
		 *     get_filter().dynamic_check(*value.first)
		 *
		 * passes.
		 *
		 * Otherwise, this function has the same effect as
		 * `std::unordered_map::insert`.
		 *
		 * @param value
		 *     value to insert
		 *
		 * @param hint
		 *     non-binding suggestion where to insert the element
		 *
		 * @returns
		 *     iterator to the newly inserted item or the item that prevented
		 *     insertion
		 *
		 */
		iterator insert(const_iterator hint, value_type&& value);

		/**
		 * @brief
		 *     Inserts a new entry into the map if it is not already present.
		 *
		 * If the map already contains an entry for a node with a different
		 * address but with the same ID, the behavior is undefined.
		 *
		 * The behavior is also undefined unless
		 *
		 *     get_filter().dynamic_check(*value.first)
		 *
		 * passes.
		 *
		 * Otherwise, this function has the same effect as
		 * `std::unordered_map::insert`.
		 *
		 * @param value
		 *     value to insert
		 *
		 * @returns
		 *     a pair holding an iterator to the newly inserted item or the
		 *     item that prevented insertion and a flag that tells whether the
		 *     item was inserted
		 *
		 */
		std::pair<iterator, bool> insert(const value_type& value);

		/**
		 * @brief
		 *     Inserts a new entry into the map if it is not already present.
		 *
		 * If the map already contains an entry for a node with a different
		 * address but with the same ID, the behavior is undefined.
		 *
		 * The behavior is also undefined unless
		 *
		 *     get_filter().dynamic_check(*value.first)
		 *
		 * passes.
		 *
		 * Otherwise, this function has the same effect as
		 * `std::unordered_map::insert`.
		 *
		 * @param value
		 *     value to insert
		 *
		 * @returns
		 *     a pair holding an iterator to the newly inserted item or the
		 *     item that prevented insertion and a flag that tells whether the
		 *     item was inserted
		 *
		 */
		std::pair<iterator, bool> insert(value_type&& value);

		/**
		 * @brief
		 *     Finds an element in a mutable map.
		 *
		 * The behavior is undefined unless
		 *
		 *     get_filter().dynamic_check(*k)
		 *
		 * passes and `k->id() == key->id()` if and only if `k == key`
		 * for all `key`s in the map.
		 *
		 * Otherwise, this function has the same effect as
		 * `std::unordered_map::find`.
		 *
		 * @param k
		 *     key to look up
		 *
		 * @returns
		 *     `iterator` `it` such that `it->first == k` or `end()`
		 *
		 */
		iterator find(const key_type& k);

		/**
		 * @brief
		 *     Finds an element in an immutable map.
		 *
		 * The behavior is undefined unless
		 *
		 *     get_filter().dynamic_check(*k)
		 *
		 * passes and `k->id() == key->id()` if and only if `&k == key`
		 * for all `key`s in the map.
		 *
		 * Otherwise, this function has the same effect as
		 * `std::unordered_map::find`.
		 *
		 * @param k
		 *     key to look up
		 *
		 * @returns
		 *     `const_iterator` `it` such that `it->first == k` or `cend()`
		 *
		 */
		const_iterator find(const key_type& k) const;

		/**
		 * @brief
		 *     Counts the occurrences of a key in the map.
		 *
		 * This function will alwayse `return` either 0 or 1.
		 *
		 * The behavior is undefined unless
		 *
		 *     get_filter().dynamic_check(*k)
		 *
		 * passes and `k->id() == key->id()` if and only if `&k == key`
		 * for all `key`s in the map.
		 *
		 * Otherwise, this function has the same effect as
		 * `std::unordered_map::count`.
		 *
		 * @param k
		 *     key to look up
		 *
		 * @returns
		 *     `(find(k) != end()) ? 1 : 0`
		 *
		 */
		size_type count(const key_type& k) const;

		/**
		 * @brief
		 *     `return`s a mutable reference to a mapped value, default
		 *     constructing one if it doesn't exist yet.
		 *
		 * This function only participates in overload resolution if
		 *
		 *     get_filter().static_check(meta::type<NodeT>{})
		 *
		 * passes.  If this is the case but
		 *
		 *     get_filter().dynamic_check(node)
		 *
		 * still doesn't pass, the behavior is undefined.
		 *
		 * If `T` is not default constructible, the behavior is also undefined.
		 *
		 * @param NodeT
		 *     static type of the node to look up
		 *
		 * @returns
		 *     mutable reference to the mapped value
		 *
		 */
		template <typename NodeT>
		apply_static_node_filter_t<NodeT, mapped_type&> operator[](NodeT&& node)
		{
			assert(get_filter().dynamic_check(node));
			return _data[&node];
		}

		/**
		 * @brief
		 *     `return`s a mutable reference to a mapped value.
		 *
		 * This function only participates in overload resolution if
		 *
		 *     get_filter().static_check(meta::type<NodeT>{})
		 *
		 * passes.  If this is the case but
		 *
		 *     get_filter().dynamic_check(node)
		 *
		 * still doesn't pass, the behavior is undefined.
		 *
		 * @param NodeT
		 *     static type of the node to look up
		 *
		 * @returns
		 *     mutable reference to the mapped value
		 *
		 * @throws std::out_of_range
		 *     if `count(&node) == 0`
		 *
		 */
		template <typename NodeT>
		apply_static_node_filter_t<NodeT, mapped_type&> at(NodeT&& node)
		{
			assert(get_filter().dynamic_check(node));
			return _data.at(&node);
		}

		/**
		 * @brief
		 *     `return`s a constant reference to a mapped value.
		 *
		 * This function only participates in overload resolution if
		 *
		 *     get_filter().static_check(meta::type<NodeT>{})
		 *
		 * passes.  If this is the case but
		 *
		 *     get_filter().dynamic_check(node)
		 *
		 * still doesn't pass, the behavior is undefined.
		 *
		 * @param NodeT
		 *     static type of the node to look up
		 *
		 * @returns
		 *     constant reference to the mapped value
		 *
		 * @throws std::out_of_range
		 *     if `count(&node) == 0`
		 *
		 */
		template <typename NodeT>
		apply_static_node_filter_t<NodeT, const mapped_type&> at(NodeT&& node) const
		{
			assert(get_filter().dynamic_check(node));
			return _data.at(&node);
		}

		/**
		 * @brief
		 *     `return`s a copy of the filter policy.
		 *
		 * @returns
		 *     filter policy object
		 *
		 */
		node_filter_type get_filter() const noexcept;

		/**
		 * @brief
		 *     `return`s a copy of the allocator.
		 *
		 * @returns
		 *     allocator object
		 *
		 */
		allocator_type get_allocator() const noexcept;

		/**
		 * @brief
		 *     `return`s a copy of the hash function.
		 *
		 * @returns
		 *     hash function object
		 *
		 */
		hasher hash_function() const noexcept;

		/**
		 * @brief
		 *     `return`s a copy of the key equality predicate.
		 *
		 * @returns
		 *     key equality predicate object
		 *
		 */
		key_equal key_eq() const noexcept;

	private:

		/** @brief Uncheked internal data storage. */
		std::unordered_map<key_type, T, hasher, key_equal, allocator_type> _data;

	};

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_ATTRIBUTE_HPP
#include "semantic/attribute.tpp"
#undef MINIJAVA_INCLUDED_FROM_ATTRIBUTE_HPP
