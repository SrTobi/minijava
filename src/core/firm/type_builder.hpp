/**
 * @file builder.hpp
 *
 * @brief
 *     Creates the IRG.
 *
 */

#pragma once

#include <cstddef>
#include <unordered_map>

#include "libfirm/firm.h"

#include "parser/ast.hpp"
#include "semantic/attribute.hpp"
#include "semantic/name_type_analysis.hpp"
#include "semantic/semantic.hpp"


namespace minijava
{

	namespace firm
	{

		struct sem_type_hash
		{
			std::size_t operator () (const sem::type &p) const {
				size_t seed = 0;
				boost::hash_combine(seed, p.info.declaration());
				boost::hash_combine(seed, p.rank);
				return seed;
			}
		};

		using type_mapping = std::unordered_map<sem::type, ir_type*, sem_type_hash>;
		using class_mapping = ast_attributes<ir_type*, ast_node_filter<ast::class_declaration> >; // TODO: use entities as values
		using field_mapping = ast_attributes<ir_entity*, ast_node_filter<ast::var_decl> >;
		using method_mapping = ast_attributes<ir_entity*, ast_node_filter<ast::method> >;

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
			ir_mode* int_mode{};

			/** @brief Unique pointer to Firm mode for MiniJava's `boolean` type.  */
			ir_mode* boolean_mode{};

			/** @brief Unique pointer to Firm type for MiniJava's `int` type.  */
			ir_type* int_type{};

			/** @brief Unique pointer to Firm type for MiniJava's `boolean` type.  */
			ir_type* boolean_type{};

		};

		struct ir_types
		{

			type_mapping typemap{};
			class_mapping classmap{};
			field_mapping fieldmap{};
			method_mapping methodmap{};

		};

		ir_types create_types(const ast::program& ast, const semantic_info& seminfo);

	}  // namespace firm

}  // namespace minijava
