/**
 * @file attribute.hpp
 *
 * @brief
 *     Attributes for AST nodes.
 *
 * TODO: Insert some musings about the generality of the `NodeFilterPolicy`
 * concept here...
 *
 * TODO: Remove everything from the interface that violates the AG idea...
 *
 */

#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "exceptions.hpp"
#include "meta/meta.hpp"
#include "parser/ast.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Generic `NodeFilterPolicy` that can be used to restrict keys to any
	 *     sub-set of AST node types.
	 *
	 * The static and dynamic check will both test whether the node in question
	 * (either statically or dynamically) derives from at least one of the
	 * `NodeTs...`.  The dynamic check additionally checks that the ID is
	 * non-zero.
	 *
	 * @tparam NodeTs
	 *     allowable base classes of key nodes
	 *
	 */
	template <typename... NodeTs>
	struct ast_node_filter
	{

		/**
		 * @brief
		 *     Tests whether the type `T` (wrapped in the tag `TypeT`) is
		 *     derived from any of the types `NodeTs...`.
		 *
		 * @tparam TypeT
		 *     any `template` that can wrap a single type
		 *
		 * @tparam T
		 *     type to check
		 *
		 * @returns
		 *     whether the type is okay
		 *
		 */
		template<template <typename> class TypeT, typename T>
		static constexpr bool static_check(TypeT<T>) noexcept
		{
			return meta::disjunction< std::is_base_of<NodeTs, T>... >{};
		}

		/**
		 * @brief
		 *     Tests whether the *dynamic* type of `node` is derived from any
		 *     of the types `NodeTs...` and `node` has a non-zero ID.
		 *
		 * @param node
		 *     AST node to check
		 *
		 * @returns
		 *     whether the node is okay
		 *
		 */
		bool dynamic_check(const ast::node& node) const noexcept;

	};

	/**
	 * @brief
	 *     Extractor for the ID of an AST node.
	 *
	 */
	struct ast_node_ptr_hash
	{
		/**
		 * @brief
		 *     `return`s the non-zero ID of the node pointed to by `nodeptr`.
		 *
		 * If `nodeptr == nullptr` or `nodeptr->id() == 0`, the behavior is
		 * undefined.
		 *
		 * @param nodeptr
		 *     pointer to AST node
		 *
		 * @returns
		 *     `nodeptr->id()`
		 *
		 */
		std::size_t operator()(const ast::node* nodeptr) const
		{
			assert((nodeptr != nullptr) && (nodeptr->id() != 0));
			return nodeptr->id();
		}
	};

	/**
	 * @brief
	 *     A data structure that associates arbitrary attributes with AST
	 *     nodes.
	 *
	 * To the extent where this makes sense, this type models the interface of
	 * a `std::unordered_map` mapping pointers to AST nodes to attributes.
	 * This is the low-level interface that can be used with STL algorithms.
	 * Unfortunately, it is not always type-safe and only `assert`s at run-time
	 * certain properties of the AST nodes that are used as keys.  Atop of (or
	 * besides) this, there are the higher-level subscript operator and `at`
	 * function that operate on references to AST nodes rather than pointers.
	 * More importantly, they will remove themselves from the overload set if
	 * the static node type does not neet the predicate defined by the
	 * `NodeFilterT` policy.  The latter may be any type that meets the
	 * `NodeFilterPolicy` as defined in the file-level documentation of this
	 * component.
	 *
	 * Unlike `std::unordered_map`, the internal memory representation of this
	 * container is completely unspecified.  Furthermore, iterators are
	 * potentially invalidated on every non-`const` quialified operation.
	 *
	 * Since the data structure has no control over the lifetime of the AST
	 * nodes it stores pointers to, it is the user's responsibility to keep the
	 * referenced AST alive for at least as long as this structure is used.
	 *
	 * AST nodes are hashed by their IDs which must be unique positive values.
	 * It can generally be assumed that the performance will be better if the
	 * numerical values of the IDs are compact, though holes are not forbidden.
	 * On the other hand, if a node with ID zero or two nodes with the same IDs
	 * but different addresses are ever passed into the same instance of an
	 * attribute map, the behavior is undefined.
	 *
	 * The policy types `NodeFilterT` and `AllocT` shall not `throw` exceptions
	 * in their copy constructors and assignment operators and (if they are
	 * default-constructible) their default constructors.  If this constraint
	 * is violated and any policy object does `throw`, the behavior is
	 * undefined.
	 *
	 * @tparam T
	 *     type of the attribute to associate with (some) AST nodes
	 *
	 * @tparam NodeFilterT
	 *     `NodeFilterPolicy` that specifies what nodes are allowed as keys
	 *
	 * @tparam AllocT
	 *     allocator to use internally (must have a `value_type` of
	 *     `std::pair<std::pair<const ast::node *const, T>`)
	 *
	 */
	template
	<
		typename T,
		typename NodeFilterT = ast_node_filter<ast::node>,
		typename AllocT = std::allocator<std::pair<const ast::node *const, T>>
	>
	class ast_attributes final : private NodeFilterT
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
			NodeFilterT::static_check(meta::type_t<std::decay_t<NodeT>>{}),
			ResultT
		>;

	public:

		/** @brief Type of the keys. */
		using key_type = const ast::node*;

		/** @brief Type of the attributes. */
		using mapped_type = T;

		/** @brief Type of a `const` key and its attribute. */
		using value_type = std::pair<const key_type, mapped_type>;

		/** @brief Hash function that `return`s the node ID. */
		using hasher = ast_node_ptr_hash;

		/** @brief Comparison function that compares node addresses. */
		using key_equal = std::equal_to<key_type>;

		/** @brief Allocator type. */
		using allocator_type = AllocT;

		/** @brief Pointer type used by the allocator. */
		using pointer = typename std::allocator_traits<allocator_type>::pointer;

		/** @brief `const` pointer type used by the allocator. */
		using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;

		/** @brief Reference type. */
		using reference = value_type&;

		/** @brief `const` reference type. */
		using const_reference = const value_type&;

		/** @brief `unsigned` integer type. */
		using size_type = std::size_t;

		/** @brief `signed` integer type. */
		using difference_type = std::ptrdiff_t;

		/** @brief Iterator over (key, value) pairs in unspecified order. */
		using iterator = typename std::unordered_map<key_type, mapped_type, hasher, key_equal, allocator_type>::iterator;

		/** @brief `const` iterator over (key, value) pairs in unspecified order. */
		using const_iterator = typename std::unordered_map<key_type, mapped_type, hasher, key_equal, allocator_type>::const_iterator;

		/** @brief `NodeFilterPolicy`. */
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
		explicit ast_attributes(const node_filter_type& filter) noexcept;

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
		explicit ast_attributes(const allocator_type& alloc) noexcept;

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
		 * @tparam NodeT
		 *     static type of the node to look up
		 *
		 * @param node
		 *     key to look up
		 *
		 * @returns
		 *     mutable reference to the mapped value
		 *
		 */
		template <typename NodeT>
		apply_static_node_filter_t<NodeT, mapped_type&> operator[](NodeT&& node)
		{
			static_assert(
				!std::is_void<NodeT>{} && std::is_default_constructible<mapped_type>{},
				"You can only use operator[] on maps with default-constructible mapped types"
			);
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
		 * @tparam NodeT
		 *     static type of the node to look up
		 *
		 * @param node
		 *     key to look up
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
		 * @tparam NodeT
		 *     static type of the node to look up
		 *
		 * @param node
		 *     key to look up
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
		 *     Assigns a mapped value.
		 *
		 * If the key is already mapped, the existing mapping is overwritten.
		 * Otherwise, a new mapping is inserted.
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
		 * @tparam NodeT
		 *     static type of the node to look up
		 *
		 * @param node
		 *     key to assign the attribute to
		 *
		 * @param attr
		 *     value of the attribute to set
		 *
		 */
		template <typename NodeT>
		apply_static_node_filter_t<NodeT> insert_or_assign(NodeT&& node, mapped_type attr)
		{
			assert(get_filter().dynamic_check(node));
			const auto pos = this->find(&node);
			if (pos == this->end()) {
				const auto status = this->insert({&node, std::move(attr)});
				assert(status.second);
				(void) status;
			} else {
				pos->second = std::move(attr);
			}
		}

		/**
		 * @brief
		 *     Inserts a new mapping but only if it doesn't exist yet.
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
		 * @tparam NodeT
		 *     static type of the node to look up
		 *
		 * @param node
		 *     key to set the attribute to
		 *
		 * @param attr
		 *     value of the attribute to set
		 *
		 * @throws std::out_of_range
		 *     if `count(&node) != 0`
		 *
		 */
		template <typename NodeT>
		apply_static_node_filter_t<NodeT> put(NodeT&& node, mapped_type attr)
		{
			assert(get_filter().dynamic_check(node));
			const auto pos = this->find(&node);
			if (pos == this->end()) {
				const auto status = this->insert({&node, std::move(attr)});
				assert(status.second);
				(void) status;
			} else {
				throw std::out_of_range{"minijava::sem::ast_attributes::put"};
			}
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
