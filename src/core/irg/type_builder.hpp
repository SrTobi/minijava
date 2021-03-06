/**
 * @file type_builder.hpp
 *
 * @brief
 *     First Firm pass to create Firm IR types and entities.
 *
 */

#pragma once

#include <cstddef>
#include <unordered_map>

#include "firm.hpp"

#include "parser/ast.hpp"
#include "semantic/attribute.hpp"
#include "semantic/name_type_analysis.hpp"
#include "semantic/semantic.hpp"


namespace minijava
{

	namespace irg
	{

		/** @brief Type mapping semantic types to Firm IR types. */
		using type_mapping = std::unordered_map<sem::type, firm::ir_type*>;

		/** @brief Type mapping runtime library builtin functions to their Firm entities. */
		using builtin_mapping = std::unordered_map<std::string, firm::ir_entity*>;

		/** @brief AST attribute type mapping class declarations to Firm IR types. */
		// TODO: Use entities instead of types as values
		using class_mapping = ast_attributes<firm::ir_type*, ast_node_filter<ast::class_declaration>>;

		/** @brief AST attribute type mapping variable declarations to Firm IR entities. */
		using field_mapping = ast_attributes<firm::ir_entity*, ast_node_filter<ast::var_decl>>;

		/** @brief AST attribute type mapping methods to Firm IR entities. */
		using method_mapping = ast_attributes<firm::ir_entity*, ast_node_filter<ast::method>>;

		/**
		 * @brief
		 *     Aggregate that holds the intermediate results of the first Firm
		 *     pass.
		 *
		 */
		struct ir_types
		{

			/**
			 * @brief
			 *     Mapping semantic types to their corresponding Firm IR types.
			 *
			 * Reference types are mapped to pointer types, not the record type
			 * of the referenced class.
			 *
			 * This map only contains types which are actually used in the
			 * program.
			 *
			 */
			type_mapping typemap{};

			/**
			 * @brief
			 *     Mapping of runtime library builtins to their Firm entities.
			 *
			 * The keys to this map are the names of the builtins.  (For
			 * example, `println` for `java.io.PrintStream.println`.)  The
			 * obvious caveat is that there are no name-spaces but this turns
			 * out to work well enough for MiniJava.  The name mangling used to
			 * derive the linker names is purposefully different because this
			 * avoids name clashed with any uder-defined symbols.
			 *
			 * The map will always contain an entry for `new`.  (This name is
			 * impossible to clash with any name defined in the AST because
			 * `new` is a reserved keyword in MiniJava.)  All other builtins
			 * are extracted from the semantic information.  enough for
			 * MiniJava.  Builtins defined in types that are never used in the
			 * program are not extracted.
			 *
			 * The builtins are always associated with firms global type, not
			 * with the type that contains the MiniJava wrapper methods.
			 *
			 */
			builtin_mapping builtins{};

			/**
			 * @brief
			 *     AST annotation mapping class declarations to their Firm IR
			 *     types.
			 *
			 * The mapped values are record and not pointer types.
			 *
			 * This map only contains classes which are actually used in the
			 * program.
			 *
			 */
			class_mapping classmap{};

			/**
			 * @brief
			 *     AST annotation mapping fields to their corresponding Firm
			 *     IR entities.
			 *
			 * This map only contains fields of classes which are actually used
			 * in the program.
			 *
			 */
			field_mapping fieldmap{};

			/**
			 * @brief
			 *     AST annotation mapping methods to their corresponding Firm
			 *     IR entities.
			 *
			 * This map only contains methods of classes which are actually used
			 * in the program.
			 *
			 */
			method_mapping methodmap{};

		};


		/**
		 * @brief
		 *     Performs the first Firm pass to create IR types and entities.
		 *
		 * If the `libfirm` was not properly initialized before calling this
		 * function, the behavior is undefined.
		 *
		 * The behavior is also undefined if `ast` is not a semantically
		 * correct program or if `seminfo` is not the result of a proper
		 * semantic analysis of `ast`.
		 *
		 * @param ast
		 *     AST of the program
		 *
		 * @param seminfo
		 *     semantic annotation for the given AST
		 *
		 * @returns
		 *     created Firm IR types and entities
		 *
		 */
		ir_types create_types(const ast::program& ast, const semantic_info& seminfo);

		/**
		 * @brief
		 *     `struct` holding Firm modes and types for MiniJava's builtin
		 *     primitive types.
		 *
		 * A default-constructed struct will hold four `nullptr`s.  In order to
		 * get an initialized object, use the `get_instance` function to obtain
		 * a reference to the singleton instance.
		 *
		 * As this `struct` merely stores four pointers, it can be freely
		 * copied.  It's the pointer members that won't change value.
		 *
		 */
		struct primitive_types
		{

			/**
			 * @brief
			 *     Creates an empty record with all members `nullptr`s.
			 *
			 */
			primitive_types() noexcept = default;

			/**
			 * @brief
			 *     Obtains a reference to the singleton instance, lazily
			 *     initializing it in a race-free manner if necessary.
			 *
			 * If `libfirm` is not initialized prior to calling this function,
			 * the behavior is undefined.
			 *
			 * @returns
			 *     reference to initialized singleton
			 *
			 */
			static const primitive_types& get_instance();

			/** @brief Unique pointer to Firm mode for MiniJava's `int` type.  */
			firm::ir_mode* int_mode{};

			/** @brief Unique pointer to Firm mode for MiniJava's `boolean` type.  */
			firm::ir_mode* boolean_mode{};

			/** @brief Unique pointer to Firm mode for MiniJava's pointer type.  */
			firm::ir_mode* pointer_mode{};

			/** @brief Unique pointer to Firm type for MiniJava's `int` type.  */
			firm::ir_type* int_type{};

			/** @brief Unique pointer to Firm type for MiniJava's `boolean` type.  */
			firm::ir_type* boolean_type{};

			/** @brief Unique pointer to Firm type for MiniJava's pointer type.  */
			firm::ir_type* pointer_type{};
		};

	}  // namespace irg

}  // namespace minijava
