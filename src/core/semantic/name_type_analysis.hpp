/**
 * @file name_type_analysis.hpp
 *
 * @brief
 *     Methods and data structures for performing name/type-analysis on the AST.
 *
 */

#pragma once

#include <cstddef>
#include <memory>
#include <iosfwd>
#include <set>
#include <vector>

#include "parser/ast.hpp"
#include "semantic/attribute.hpp"
#include "semantic/type_info.hpp"
#include "symbol/symbol.hpp"


namespace minijava
{

	namespace sem
	{

		/**
		 * @brief
		 *     (Semantic) type
		 *
		 */
		struct type
		{
			/**
			 * @brief
			 *     Constructs a new (semantic) type.
			 *
			 * @param info
			 *     meta information about this type's basic type
			 *
			 * @param rank
			 *     rank of the array or 0 if this type is not an array
			 *
			 */
			constexpr type(const basic_type_info info, const std::size_t rank) noexcept
					: info{info}, rank{rank}
			{
			}

			/** @brief Meta information about this type's basic type. */
			basic_type_info info;

			/** @brief Rank of the array or 0 if this type is not an array. */
			std::size_t rank;
		};

		/**
		 * @brief
		 *     Compares two `type`s for equality.
		 *
		 * @param lhs
		 *     first `type` to compare
		 *
		 * @param rhs
		 *     second `type` to compare
		 *
		 * @returns
		 *     whether `lhs` and `rhs` have same basic type and rank
		 *
		 */
		constexpr bool operator==(const type lhs, const type rhs) noexcept
		{
			return ((lhs.info == rhs.info) && (lhs.rank == rhs.rank));
		}

		/**
		 * @brief
		 *     Compares two `type`s for inequality.
		 *
		 * @param lhs
		 *     first `type` to compare
		 *
		 * @param rhs
		 *     second `type` to compare
		 *
		 * @returns
		 *     `!(lhs == rhs)`
		 *
		 */
		constexpr bool operator!=(const type lhs, const type rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/**
		 * @brief
		 *     Inserts a human-readable representation of a `type` into a
		 *     stream.
		 *
		 * The written output might not be valid MiniJava.
		 *
		 * @param os
		 *     stream to write to
		 *
		 * @param typ
		 *     `type` to write
		 *
		 * @returns
		 *     reference to `os`
		 *
		 */
		std::ostream& operator<<(std::ostream& os, type typ);

		/** @brief Type of the vector containing all globals. */
		using globals_vector = std::vector<std::unique_ptr<ast::var_decl>>;

		/** @brief Type mapping typed AST nodes to type definitions. */
		using type_attributes = ast_attributes<
			type,
			ast_node_filter<ast::expression, ast::var_decl, ast::method>
		>;

		/** @brief Type mapping `method` nodes to sets of `var_decl` nodes. */
		using locals_attributes = ast_attributes<
			std::set<const ast::var_decl*>,
			ast_node_filter<ast::method>
		>;

		/** @brief Type mapping `var_access` nodes to `var_decl` nodes. */
		using vardecl_attributes = ast_attributes<
			const ast::var_decl*,
			ast_node_filter<ast::variable_access>
		>;

		/** @brief Type mapping `var_access` and `array_access` nodes to `var_decl` nodes. */
		using method_attributes = ast_attributes<
			const ast::instance_method*,
			ast_node_filter<ast::method_invocation>
		>;


		/**
		 * @brief
		 *     Performs name and type analysis on a program and saves the
		 *     results as annotations.
		 *
		 * This function will check the following properties of the program and
		 * `throw` an exception if any of them is violated.  If this happens,
		 * it is unspecified what values have been written into the annotation
		 * containers.
		 *
		 * - There are no two methods in any `class` with pairwise identical
		 *   names.
		 *
		 * - The only `static` method is `main`.
		 *
		 * - There are exactly `int(expect_main)` `main` methods in the whole
		 *   program.
		 *
		 * - All declarations refer to known types and no arrays of `void` are
		 *   declared.  `void` is only used as method `return` type.
		 *
		 * - All used identifiers are declared and unambiguous as per the
		 *   language rules.  (No conflicting identifiers, no shadowing of
		 *   local variables.)
		 *
		 * - All expressions in the program are well-typed as per the language
		 *   rules.
		 *
		 * - The number and types of the expressions in argument lists to
		 *   method calls match the method signature and the type of the
		 *   expression in a `return` statement matches the type of the method.
		 *
		 * - The body of the `main` method (if any) doesn't refer to fields or
         *   instance methods of the containing `class` and the formal
         *   parameter of `main` is never accessed.  (For the purpose of
         *   shadowing, it is trated like a local variable, but it has no
         *   declaration, no type and won't be included in the set of locals.
         *   This is because MiniJava is a messy language.)
		 *
		 * If there are multiple errors in the program, it is unspecified which
		 * error will be reported.
		 *
		 * If there are class definitions in the `ast` that have no entry in
		 * `classes`, the behavior is undefined.  On the other hand, `classes`
		 * may contain types that are not defined in `ast`.  For the purpose of
		 * the type analysis performed by this function, those types are
		 * treated as if they were defined in `ast`.
		 *
		 * The behavior is further undefined if `globals` mentions types that
		 * are not found in `classes`.
		 *
		 * The `ast_attributes` containers that are passed in to be populated
		 * with results may already contain some entries, but only for
		 * non-conflicting nodes.  If a container already sets an attribute on
		 * a node that this analysis would also set, the behavior is undefined.
		 *
		 * @param ast
		 *     AST to analyze
		 *
		 * @param expect_main
		 *     if `true`, exactly one `main` method is expected to be found in
		 *     the AST; otherwise no `main` method is expected
		 *
		 * @param classes
		 *     all class types in the program
		 *
		 * @param globals
		 *     implicit global variables in the program
		 *
		 * @param type_annotations
		 *     data structure to populate with extracted type annotations
		 *
		 * @param locals_annotations
		 *     data structure to populate with extracted local variable
		 *     declarations
		 *
		 * @param vardecl_annotations
		 *     data structure to populate with extracted declaration pointer
		 *
		 * @param method_annotations
		 *     data structure to populate with extracted declaration pointers
		 *
		 * @throws semantic_error
		 *     if the program fails any of the checks listed above
		 *
		 */
		void perform_name_type_analysis(const ast::program&      ast,
		                                bool                     expect_main,
		                                const class_definitions& classes,
		                                const globals_vector&    globals,
		                                type_attributes&         type_annotations,
		                                locals_attributes&       locals_annotations,
		                                vardecl_attributes&      vardecl_annotations,
		                                method_attributes&       method_annotations);

	}  // namespace sem

}  // namespace minijava
