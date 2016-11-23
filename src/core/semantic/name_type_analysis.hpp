/**
 * @file name_type_analysis.hpp
 *
 * @brief
 *     Methods and data structures for performing name/type-analysis on the AST.
 *
 */

#pragma once

#include <cstddef>
#include <unordered_map>
#include <unordered_set>

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
			type(basic_type_info info, std::size_t rank)
					: info{info}, rank{rank} {}

			/** @brief meta information about this type's basic type */
			const basic_type_info info;

			/** @brief rank of the array or 0 if this type is not an array */
			const std::size_t rank;
		};

		/**
		 * @brief
		 *     Type of the map containing the global variables.
		 */
		using globals_map = std::unordered_map<symbol, symbol>;

		/**
		 * @brief
		 *     Type of the type annotations on expressions and other AST nodes.
		 */
		using type_attributes = ast_attributes<type, ast_node_filter<ast::expression, ast::var_decl, ast::method>>;

		/**
		 * @brief
		 *     Type of the local variable list annotations on method nodes.
		 */
		using locals_attributes = ast_attributes<std::unordered_set<const ast::var_decl*>, ast_node_filter<ast::method>>;

		/**
		 * @brief
		 *     Type of the annotations pointing to variable declarations.
		 */
		using vardecl_attributes = ast_attributes<const ast::var_decl*, ast_node_filter<ast::array_access, ast::variable_access>>;

		/**
		 * @brief
		 *     Type of the annotations pointing to method declarations.
		 */
		using method_attributes = ast_attributes<const ast::instance_method*, ast_node_filter<ast::method_invocation>>;

		/**
		 * @brief
		 *     Analyzes the types of fields, parameters and return types in the
		 *     given AST.
		 *
		 * @param ast
		 *     AST to analyze
		 *
		 * @param types
		 *     type annotation data structure to use
		 *
		 */
		void perform_shallow_type_analysis(const ast::program& ast, type_attributes& type_annotations);

		/**
		 * @brief
		 *     Performs a full name/type analysis on the given AST
		 *
		 * @param ast
		 *     AST to analyze
		 *
		 * @param globals
		 *     implicit global variables in the program
		 *
		 * @param types
		 *     type annotation data structure to use
		 *
		 * @param locals
		 *     locals annotation data structure to use
		 *
		 * @param var_decls
		 *     variable declaration annotation data structure to use
		 *
		 * @param method_decls
		 *     variable declaration annotation data structure to use
		 *
		 */
		void perform_full_name_type_analysis(const ast::program& ast,
											 const globals_map&  globals,
											 type_attributes&    type_annotations,
											 locals_attributes&  locals_annotations,
											 vardecl_attributes& vardecl_annotations,
											 method_attributes&  method_annotations);

	}  // namespace sem

}  // namespace minijava
