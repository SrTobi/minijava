/**
 * @file semantic.hpp
 *
 * @brief
 *     Public interface for semantic analysis.
 *
 */

#pragma once

#include <cstdint>
#include <memory>
#include <utility>

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "semantic/constant.hpp"
#include "semantic/name_type_analysis.hpp"
#include "semantic/type_info.hpp"
#include "symbol/symbol_pool.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Aggregate for all semantic information collected during the semantic
	 *     analysis of the program.
	 *
	 * Among other information, this data structure contains the AST
	 * annotations.  It also contains as a `private` member an immutable copy
	 * of the AST that defines the builtin class types such that references to
	 * them from other annotations remain valid as long as the `semantic_info`
	 * object exists.
	 *
	 */
	class semantic_info final
	{

	public:

		/** @brief Type mapping class names to type definitions. */
		using class_definitions = sem::class_definitions;

		/** @brief Type mapping typed AST nodes to type definitions. */
		using type_attributes = sem::type_attributes;

		/** @brief Type mapping `method` nodes to a vector of `var_decl` nodes. */
		using locals_attributes = sem::locals_attributes;

		/** @brief Type mapping `var_access` and `array_access` nodes to `var_decl` nodes. */
		using vardecl_attributes = sem::vardecl_attributes;

		/** @brief Type mapping `method_invocation` nodes to `instance_method` nodes. */
		using method_attributes = sem::method_attributes;

		/** @brief Type mapping `expression` nodes to integers. */
		using const_attributes = sem::const_attributes;

		/**
		 * @brief
		 *     Constructs an immutable semantic information aggregate.
		 *
		 * Clients are not supposed to use this constructor; they should use
		 * `check_program` instead.
		 *
		 * The behavior is undefined unless the given parameters together form
		 * a consistent analysis.  The behavior is undefined in particular if
		 * `builtin_ast.get() == nullptr` or if `globals` constains a
		 * `nullptr`.
		 *
		 * @param classes
		 *     classes (built-in and user-defined) in the program
		 *
		 * @param type_annotations
		 *     mapping of typed AST nodes to the definition of their type
		 *
		 * @param locals_annotations
		 *     mapping of `method` nodes to the a vector of their local variable
		 *     declarations
		 *
		 * @param vardecl_annotations
		 *     mapping of `var_access` nodes to the node that declares the used
		 *     identifier
		 *
		 * @param method_annotations
		 *     mapping of `method_invocation` nodes to the node that defines
		 *     the called method
		 *
		 * @param const_annotations
		 *     mapping of constant `expression` nodes to their value
		 *
		 * @param builtin_ast
		 *     AST with definitions of built-in classes
		 *
		 * @param globals
		 *     global variables
		 *
		 */
		semantic_info(class_definitions classes,
		              type_attributes type_annotations,
		              locals_attributes locals_annotations,
		              vardecl_attributes vardecl_annotations,
		              method_attributes method_annotations,
		              const_attributes const_annotations,
		              std::unique_ptr<ast::program> builtin_ast,
		              sem::globals_vector globals);

		/**
		 * @brief
		 *     `return`s the mapping from class names (builtin and
		 *     user-defined) to their definitions.
		 *
		 * @returns
		 *     `const` reference to class definitions mapping
		 *
		 */
		const class_definitions& classes() const noexcept
		{
			return _classes;
		}

		/**
		 * @brief
		 *     `return`s the mapping from typed AST nodes to their type
		 *     definitions.
		 *
		 * For example, the variable access node `foo` will be mapped to the
		 * definition of `foo`'s type.
		 *
		 * All `var_decl`, `method` and `expression` nodes in the AST will be
		 * mapped for if their type cannot be determined, the program is
		 * ill-formed and will be rejected.  The type of a `var_decl` node is
		 * the type of the declared identifier.  The type of a `method` node is
		 * the `return` type of the declared method.  (Because MiniJava is more
		 * like Fortran than like Haskell.)
		 *
		 * @returns
		 *     `const` reference to type annotations
		 *
		 */
		const type_attributes& type_annotations() const noexcept
		{
			return _type_annotations;
		}

		/**
		 * @brief
		 *     `return`s a mapping from `method` nodes to a vector of `var_decl`
		 *     nodes of their local variable declarations.
		 *
		 * For example, the `instance_method` node for the following method
		 *
		 *     int meanSquared(int a, int b) {
		 *                     ^^^^^  ^^^^^
		 *         int c = (a + b) / 2;
		 *         ^^^^^
		 *         return c * c;
		 *     }
		 *
		 * will be mapped to the `var_decl` nodes for the declarations of `a`,
		 * `b` and `c`.  Note that the `return` value is not a declared
		 * variable.  Neither are the unnamed intermediary results.  This vector
		 * is flattened over all nested scopes a method body might contain.
		 * The vector contains at first all params in their declaring order
		 * without the implicit this pointer
		 *
		 * All `instance_method` and `main_method` nodes will have this
		 * attribute set.
		 *
		 * @returns
		 *     `const` reference to local variable annotations
		 *
		 */
		const locals_attributes& locals_annotations() const noexcept
		{
			return _locals_annotations;
		}

		/**
		 * @brief
		 *     `return`s the mapping from `var_access` node to the `var_decl`
		 *     node that declares the used identifier.
		 *
		 * For example, in this snippet (which invokes undefined behavior at
		 * run-time due to using an unitialized variable, but never mind that),
		 *
		 *         +--------------+
		 *         |              |
		 *         V              |
		 *     int a;             |
		 *     System.out.println(a);
		 *
		 * the `var_access` node for the usage of `a` on the second line will
		 * be mapped to the `var_decl` node for the declaration of `a` on the
		 * first line.
		 *
		 * All `var_access` nodes will be mapped for if the point of declaration
		 * of the referenced identifier cannot be determined, the program is
		 * ill-formed and will be rejected.
		 *
		 * @returns
		 *     `const` reference to variable declaration annotations
		 *
		 */
		const vardecl_attributes& vardecl_annotations() const noexcept
		{
			return _vardecl_annotations;
		}

		/**
		 * @brief
		 *     `return`s the mapping from `method_invocation` nodes to the
		 *     `instance_method` node of the called method.
		 *
		 * For example, in the following recursive snippet,
		 *
		 *         +--------------+
		 *         |              |
		 *         V              |
		 *     int fac(int n) {   |
		 *         if (n == 0) {  |
		 *             return 1;  |
		 *         } else {       |
		 *             return n * fac(n - 1);
		 *         }
		 *     }
		 *
		 * the `method_invocation` node for the call to `fac` on the fifth line
		 * will be mapped to the `instance_method` node that defines `fac`.
		 *
		 * All `method_invocation` nodes will be mapped for if the definition
		 * of the called method cannot be determined, the program is ill-formed
		 * and will be rejected.  The mapped type is `instance_method` because
		 * a `main_method` cannot be called.
		 *
		 * @returns
		 *     `const` reference to method annotations
		 *
		 */
		const method_attributes& method_annotations() const noexcept
		{
			return _method_annotations;
		}

		/**
		 * @brief
		 *     `return`s a mapping from `expression` nodes to their constant
		 *     value (if it is known).
		 *
		 * For example, in the following snippet
		 *
		 *     int a = 5;
		 *     int b = a * (1 + 2);
		 *
		 * the `integer_constant` nodes will be mapped to the constants 5, 1
		 * and 2 respectively and the `binary_expression` node for the addition
		 * on the second line will be mapped to the constant 3.  The
		 * `binary_expression` for the multiplication node will not be mapped
		 * because no constant propagation across expression boundaries is
		 * performed.
		 *
		 * Assignment expression are never mapped, even if the value of the
		 * right-hand side is known.  If evaluating a constant expression would
		 * invoke undefined behavior, it is considered undefined and no
		 * constant is mapped.
		 *
		 * @returns
		 *     `const` reference to constant annotations
		 *
		 */
		const const_attributes& const_annotations() const noexcept
		{
			return _const_annotations;
		}

		/**
		 * @brief
		 *     Checks whether the given declaration declares global variable.
		 *
		 * If `declaration` does not point to an existing declaration (in
		 * particular, if `declaration == nullptr`), the behavior is undefined.
		 *
		 * @param declaration
		 *     declaration to check
		 *
		 * @return
		 *     `true` if the given declaration is one of the global variable
		 *     declarations, `false` otherwise
		 *
		 */
		bool is_global(const ast::var_decl* declaration) const;

	private:

		/** @brief Classes (built-in and user-defined) in the program. */
		class_definitions _classes;

		/** @brief Mapping of typed AST nodes to the definition of their type. */
		type_attributes _type_annotations;

		/** @brief Mapping of `method` nodes to the vector of their local variable declarations. */
		locals_attributes _locals_annotations;

		/** @brief Mapping of `var_access` nodes to the node that declare the used identifier. */
		vardecl_attributes _vardecl_annotations;

		/** @brief Mapping of `method_invocation` nodes to the node that defines the called method. */
		method_attributes _method_annotations;

		/** @brief Mapping of constant `expression` nodes to their value. */
		const_attributes _const_annotations;

		/** @brief AST with definitions of built-in classes. */
		std::unique_ptr<ast::program> _builtin_ast;

		/** @brief global variables, sorted by memory address of the AST node */
		sem::globals_vector _globals;

	};

	/**
	 * @brief
	 *     Checks the semantic validity of a program and extracts semantic
	 *     information.
	 *
	 * This function will use the provided pool and factory to create
	 * additional identifiers and AST nodes for builtin types.  If the pool is
	 * different from the one that created the symbols used in the AST or if
	 * the factory will create nodes with IDs that are already in the given
	 * AST, the behavior is undefined.
	 *
	 * @tparam PoolT
	 *     symbol pool type
	 *
	 * @param ast
	 *     program AST
	 *
	 * @param pool
	 *     symbol pool that was used for identifiers in the AST
	 *
	 * @param factory
	 *     factory that was used to create the original AST
	 *
	 * @returns
	 *     semantic information about valid programs
	 *
	 * @throws semantic_error
	 *     if the AST does not describe a valid MiniJava program
	 *
	 */
	template <typename PoolT>
	semantic_info check_program(const ast::program& ast,
	                            PoolT& pool,
	                            ast_factory& factory);


	/**
	 * @brief
	 *     Details of semantic analysis.
	 *
	 * Unlike `detail`, the features provided in this `namespace` are open for
	 * external use but the intention is that knowing about them will not be
	 * required in order to use this sub-system of the compiler.
	 *
	 */
	namespace sem {}

}  // namespace minijava

#define MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
#include "semantic/semantic.tpp"
#undef MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
