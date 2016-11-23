/**
 * @file semantic.hpp
 *
 * @brief
 *     Public interface for semantic analysis.
 *
 */

#pragma once

#include <cstdint>

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "semantic/attribute.hpp"
#include "semantic/name_type_analysis.hpp"
#include "semantic/type_info.hpp"
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
	class semantic_info
	{

	public:

		/**
		 * @brief
		 *     Type of the map containing all type definitions.
		 */
		using type_definitions = sem::type_definitions;

		/**
		 * @brief
		 *     Type of the type annotations on expressions and other AST nodes.
		 */
		using type_annotations = sem::type_annotations;

		/**
		 * @brief
		 *     Type of the local variable list annotations on method nodes.
		 */
		using locals = sem::locals;

		/**
		 * @brief
		 *     Type of the annotations pointing to variable declarations.
		 */
		using var_declarations = sem::var_declarations;

		/**
		 * @brief
		 *     Type of the annotations pointing to method declarations.
		 */
		using method_declarations = sem::method_declarations;

		/**
		 * @brief
		 *     Type of the constant value expression annotations.
		 */
		// FIXME: move to constant.hpp
		using constant_values = ast_attributes<std::int32_t, ast_node_filter<ast::expression>>;

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
		 * @param builtin_ast
		 *     AST containing definitions of the built-in reference types
		 *
		 */
		semantic_info(type_definitions types, type_annotations ast_types,
		              locals locals, var_declarations var_decls,
		              method_declarations method_decls,
					  constant_values constants,
					  std::unique_ptr<ast::program> builtin_ast)
				: _types{std::move(types)}, _ast_types{std::move(ast_types)},
				  _locals{std::move(locals)}, _var_decls{std::move(var_decls)},
				  _method_decls{std::move(method_decls)},
				  _constants{std::move(constants)},
				  _builtin_ast{std::move(builtin_ast)}
		{
			assert (_builtin_ast);
		}

		// FIXME: getter for all except _builtin_ast

	private:

		/** @brief types in the program */
		type_definitions _types;

		/** @brief type annotations */
		type_annotations _ast_types;

		/** @brief local variables annotations of methods */
		locals _locals;

		/** @brief variable declaration pointer annotations */
		var_declarations _var_decls;

		/** @brief method declaration pointer annotations */
		method_declarations _method_decls;

		/** @brief constant value annotations */
		constant_values _constants;

		/** @brief AST containing definitions of the built-in reference types */
		// retained here to make sure it doesn't go out of scope prematurely
		std::unique_ptr<ast::program> _builtin_ast;

	};

	/**
	 * @brief
	 *     Checks the semantic validity of the given program.
	 *
	 * @param ast
	 *     program AST
	 *
	 * @param pool
	 *     symbol pool to use for built-in identifiers
	 *
	 * @param factory
	 *     AST factory which was used to create the program AST
	 *
	 * @return
	 *     semantic information, including AST annotations
	 *
	 * @throws semantic_error
	 *     if the program is semantically invalid
	 *
	 */
	template<typename AllocT>
	semantic_info check_program(const ast::program& ast,
							    symbol_pool<AllocT>& pool,
								ast_factory& factory);

	// FIXME: remove
	template<typename AllocT>
	void analyze_ast(const ast::program& ast, symbol_pool<AllocT>& pool);
}

#define MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
#include "semantic/semantic.tpp"
#undef MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
