/**
 * @file ast_factory.hpp
 *
 * @brief
 *     Factories for ASTs.
 *
 */

#pragma once

#include <memory>

#include "parser/ast.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     A builder for AST nodes.
	 *
	 * An `ast_builder` is nothing more than a collection of attributes that
	 * should be set on the to-be-created node.  Various member functions allow
	 * setting these attributes.  This is a one-shot thing.  Once an attribute
	 * has been set, it cannot be re-set.
	 *
	 * While this `class` can also be used in isolation, it is most useful when
	 * used in combination with `ast_factory`.
	 *
	 * `ast_builder`s are theoretically capable of creating many AST nodes.
	 * However, the intended use-case is to create one node per builder.
	 *
	 * The behavior is undefined unless `NodeT` is derived from `ast::node`.
	 *
	 * @tparam NodeT
	 *     the type of AST node this builder should create
	 *
	 */
	template <typename NodeT>
	class ast_builder final
	{

		static_assert(
			std::is_base_of<ast::node, NodeT>{},
			"An ast_builder can only build AST nodes"
		);

	public:

		/**
		 * @brief
		 *     Creates an `ast_builder` that will create nodes with ID `id`.
		 *
		 * @param id
		 *     ID for the nodes that will be created
		 *
		 */
		ast_builder(const std::size_t id = 0) noexcept;

		/**
		 * @brief
		 *     Associates a line number with the to-be-created `node`.
		 *
		 * This function may be called at most once and only to set a non-zero
		 * line number.  Attempting to re-set the line number that was set
		 * before or to set the line number to zero will result in undefined
		 * behavior.
		 *
		 * @param line
		 *     line number to associate with the `node`
		 *
		 * @returns
		 *     reference to `*this`
		 *
		 */
		ast_builder& at_line(std::size_t line);

		/**
		 * @brief
		 *     Associates a column number with the to-be-created `node`.
		 *
		 * This function may be called at most once and only to set a non-zero
		 * column number.  Attempting to re-set the column number that was set
		 * before or to set the column number to zero will result in undefined
		 * behavior.
		 *
		 * @param column
		 *     column number to associate with the `node`
		 *
		 * @returns
		 *     reference to `*this`
		 *
		 */
		ast_builder& at_column(std::size_t column);

		/**
		 * @brief
		 *     Creates a `node` from the given arguments and sets its
		 *     attributes to the values set previously.
		 *
		 * This function will only participate in overload resolution if a
		 * `node` of type `NodeT` can be constructed from arguments of type
		 * `ArgTs...`.
		 *
		 * @tparam ArgTs
		 *     types of constructor parameters
		 *
		 * @param args
		 *     constructor parameters
		 *
		 * @returns
		 *     new `node`
		 *
		 */
		template <typename... ArgTs>
		std::enable_if_t
		<
			std::is_constructible<NodeT, ArgTs...>{},
			std::unique_ptr<NodeT>
		>
		operator()(ArgTs&&... args) const;

	private:

		/** @brief Aggregated attributes to be set on the new `node`. */
		ast::node::mutator _mutator{};

	};


	/**
	 * @brief
	 *     Factory for AST nodes.
	 *
	 * The primary purpose of this `class` is to create AST nodes with
	 * successive non-zero IDs.
	 *
	 * IDs are managed individually by each instance of this `class`.  The idea
	 * is to use one `ast_factory` per AST.
	 *
	 */
	class ast_factory final
	{
	public:

		/**
		 * @brief
		 *     Creates a factory that will create nodes with successive IDs
		 *     starting at `lastid + 1`.
		 *
		 * @param lastid
		 *     one before the next ID to use
		 *
		 */
		explicit ast_factory(const std::size_t lastid = 0) noexcept;

		/**
		 * @brief
		 *     `return`s an `ast_builder` that will create `node`s of type
		 *     `NodeT` with the next Id.
		 *
		 * Obviously, the `ast_builder` should only be used to create exactly
		 * one `node` or the IDs will be ambigous.
		 *
		 * This function only participates in overload resolution if `NodeT` is
		 * derived from `ast::node`.
		 *
		 * @tparam NodeT
		 *     type of `node` to create
		 *
		 * @tparam ForbiddenTs
		 *     must no be used
		 *
		 * @param forbidden
		 *     must no be used
		 *
		 */
		template <typename NodeT, typename... ForbiddenTs>
		std::enable_if_t<std::is_base_of<ast::node, NodeT>{}, ast_builder<NodeT>>
		make(ForbiddenTs&&... forbidden);

		/**
		 * @brief
		 *     `return`s the current internal ID.
		 *
		 * The `return`ed value is the value that was passed to the constructor
		 * as `lastid` plus the number of times `make` was called since.
		 *
		 * @returns
		 *     current internal ID
		 *
		 */
		std::size_t id() const noexcept;

	private:

		/** @brief Last ID that was used. */
		std::size_t _id{};

	};  // class ast_factory

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_PARSER_AST_FACTORY_HPP
#include "parser/ast_factory.tpp"
#undef MINIJAVA_INCLUDED_FROM_PARSER_AST_FACTORY_HPP
