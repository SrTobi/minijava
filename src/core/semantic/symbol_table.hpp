/**
 * @file symbol_table.hpp
 *
 * @brief
 *     Namespaces and symbol visibilities.
 *
 */

#pragma once

#include <unordered_map>

#include "parser/ast.hpp"
#include "symbol/symbol.hpp"


namespace minijava
{

	namespace sem
	{

		/**
		 * @brief
		 *     Nested name-spaces of visible symbol definitions.
		 *
		 * The `symbol_table` maintains a stack of *scopes*.  The top-most
		 * scope is the one that was least recently opened.  Each scope has a
		 * mapping of identifiers to definition points.  At each point, it is
		 * possible to ask for a symbol what the currently visible definition
		 * is, or if there is none at all.  Furthermore, new definitions can be
		 * added to the current scope and those will be checked whether they
		 * conflict with any previous definition.  Each scope has an attribute
		 * that defines whether nested scopes may or may not shadow its
		 * definitions.  A `symbol_table` must have at least one scope pushed
		 * in order for the operations on definitions to be well-defined.
		 *
		 */
		class symbol_table final
		{

			/**
			 * @brief
			 *     Name-space for symbol definitions.
			 *
			 */
			struct scope
			{
				/** @brief Mapping of locally defined symbols to their point of definition. */
				std::unordered_map<symbol, const ast::var_decl*> defs{};

				/** @brief Indicator whether nested scopes may shadow definitions from this scope. */
				bool may_shadow{};
			};

		public:

			/**
			 * @brief
			 *     Creates an empty symbol table with no initial scope.
			 *
			 * Before an initial scope is pushed, all operations other than
			 * reassignment and destruction will invoke undefined behavior.
			 *
			 */
			symbol_table() noexcept;

			/**
			 * @brief
			 *     `return`s the currently visible definition of a symbol, if
			 *     it exists, or else `nullptr`.
			 *
			 * Note that this function may find a definition even if
			 * %get_conflicting_definitions reports no conflicts.
			 *
			 * The behavior is undefined unless at least one scope has been
			 * pushed.
			 *
			 * @param name
			 *     symbol to look up
			 *
			 * @returns
			 *     pointer to definition or `nullptr` if no definition exists
			 *
			 */
			const ast::var_decl* lookup(const symbol name) const;

			/**
			 * @brief
			 *     Checks whether the symbol `name` (if defined in the current
			 *     scope) would conflict with any previous definitions.
			 *
			 * A symbol may not be defined if there is a definition of the same
			 * symbol in the same scope or in a parent scope that has the
			 * `may_shadow` attribute set to `false`.  If such a conflicting
			 * definition is found, a pointer to it is `return`ed.
			 *
			 * The behavior is undefined unless at least one scope has been
			 * pushed.
			 *
			 * @param name
			 *     symbol to check
			 *
			 * @returns
			 *     pointer to conflicting definition or `nullptr` if no such
			 *     definition exists
			 *
			 */
			const ast::var_decl* get_conflicting_definitions(symbol name) const;

			/**
			 * @brief
			 *     Adds a definition to the current scope.
			 *
			 * If there is a conflicting definition, an exception is `throw`n
			 * and the scope is left unchanged.
			 *
			 * The behavior is undefined unless at least one scope has been
			 * pushed.
			 *
			 * @param def
			 *     pointer to AST node with definition
			 *
			 * @throws semantic_error
			 *     if there is a conflicting definition
			 *
			 */
			void add_def(const ast::var_decl* def);

			/**
			 * @brief
			 *     Pushes a new (initially empty) scope.
			 *
			 * @param may_shadow
			 *     whether nested scopes may shadow definitions from this scope
			 *
			 */
			void enter_scope(bool may_shadow = false);

			/**
			 * @brief
			 *     Pops the most recent (most nested) scope.
			 *
			 * The behavior is undefined unless at least one scope has been
			 * pushed.  If this operation causes the last scope to be popped,
			 * then any subsequent operations other than reassignment or
			 * destruction will trigger undefined behavior, unless another
			 * scope is pushed again.
			 *
			 */
			void leave_scope();

			/**
			 * @brief
			 *     `return`s the current depth of nested scopes.
			 *
			 * @returns
			 *     level of nested scopes
			 *
			 */
			std::size_t depth() const noexcept;

		private:

			/** @brief Stack of nested scopes (top is most nested). */
			std::vector<scope> _nested_scopes{};

		};  // class symbol_table

	}  // namespace sem

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_SEMANTIC_SYMBOL_TABLE_HPP
#include "semantic/symbol_table.tpp"
#undef MINIJAVA_INCLUDED_FROM_SEMANTIC_SYMBOL_TABLE_HPP
