/**
 * @file analyze.hpp
 *
 * @brief
 *     Public interface for semantic analysis.
 *
 */

#pragma once

#include <cstdint>
#include <unordered_set>

#include "parser/ast.hpp"
#include "semantic/attribute.hpp"
#include "symbol/symbol_pool.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Container for all semantic information collected during the semantic
	 *     analysis of the program.
	 *
	 * Among other information, this data structure contains the AST
	 * annotations.
	 *
	 */
	// FIXME: define constraints for attribute types
	struct semantic_info
	{
		/**
		 * @brief
		 *     Type of the map containing all type definitions.
		 */
		// FIXME: map string -> type_info
		using type_definitions = int;

		/**
		 * @brief
		 *     Type of the type annotations on expressions and other AST nodes.
		 */
		// FIXME: type_info + rank data structure
		using type_annotations = ast_attributes<int>;

		/**
		 * @brief
		 *     Type of the local variable list annotations on method nodes.
		 */
		using locals = ast_attributes<std::unordered_set<const ast::var_decl*>>;

		/**
		 * @brief
		 *     Type of the annotations pointing to variable declarations.
		 */
		using var_declarations = ast_attributes<const ast::var_decl*>;

		/**
		 * @brief
		 *     Type of the annotations pointing to method declarations.
		 */
		using method_declarations = ast_attributes<const ast::instance_method*>;

		/**
		 * @brief
		 *     Type of the constant value expression annotations.
		 */
		using constant_values = ast_attributes<std::int32_t>;

		/**
		 * @brief
		 *     Constructs a semantic information container.
		 *
		 * @param types
		 *     types in the program
		 *
		 * @param ast_types
		 *     type annotations
		 *
		 * @param locals
		 *     local variables annotations of methods
		 *
		 * @param var_decls
		 *     variable declaration pointer annotations
		 *
		 * @param method_decls
		 *     method declaration pointer annotations
		 *
		 * @param constants
		 *     constant value annotations
		 *
		 */
		semantic_info(type_definitions types, type_annotations ast_types,
		              locals locals, var_declarations var_decls,
		              method_declarations method_decls,
					  constant_values constants)
				: _types{std::move(types)}, _ast_types{std::move(ast_types)},
				  _locals{std::move(locals)}, _var_decls{std::move(var_decls)},
				  _method_decls{std::move(method_decls)},
				  _constants{std::move(constants)}
		{}

		/** @brief types in the program **/
		type_definitions _types;

		/** @brief type annotations **/
		type_annotations _ast_types;

		/** @brief local variables annotations of methods **/
		locals _locals;

		/** @brief variable declaration pointer annotations **/
		var_declarations _var_decls;

		/** @brief method declaration pointer annotations **/
		method_declarations _method_decls;

		/** @brief constant value annotations **/
		constant_values _constants;
	};

	/**
	 * @brief
	 *     Checks the semantic validity of the given program.
	 *
	 * @param ast
	 *     program ast
	 *
	 * @param pool
	 *     symbol pool used for identifiers
	 *
	 * @return
	 *     semantic information, including AST annotations
	 *
	 */
	template<typename AllocT>
	semantic_info check_program(const ast::program& ast,
							    symbol_pool<AllocT>& pool);

	// FIXME: remove
	template<typename AllocT>
	void analyze_ast(const ast::program& ast, symbol_pool<AllocT>& pool);
}

#define MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
#include "semantic/semantic.tpp"
#undef MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
