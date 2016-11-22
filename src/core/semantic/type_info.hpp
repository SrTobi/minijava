/**
 * @file type_info.hpp
 *
 * @brief
 *     Methods and data structures for extracting type information from the AST.
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "parser/ast.hpp"

namespace minijava
{
	namespace sem
	{
		class basic_type_info;

		/**
		 * @brief
		 *     Type of the map containing all type definitions.
		 *
		 * This is not implemented as an `ast_attribute` since some types are
		 * not represented in the program's AST.
		 *
		 */
		using type_definitions = std::unordered_map<std::string, basic_type_info>;

		/**
		 * @brief
		 *     Meta information describing a basic, i.e. non-array, type.
		 */
		class basic_type_info
		{

		private:

			/** @brief mask indicating that the type is built-in */
			static const std::uint8_t builtin_mask{1};

			/** @brief mask indicating that the type is primitive */
			static const std::uint8_t primitive_mask{1 << 1};

			/** @brief mask indicating that the type is instantiable */
			static const std::uint8_t instantiable_mask{1 << 2};

			/** @brief type categories */
			enum class category : std::uint8_t {
				user_defined = instantiable_mask,
				builtin_class = builtin_mask | instantiable_mask,
				null_type = builtin_mask,
				void_type = builtin_mask | primitive_mask,
				int_type = builtin_mask | primitive_mask | instantiable_mask,
				boolean_type = builtin_mask | primitive_mask | instantiable_mask
						| (1 << 7)
			};

		public:

			/**
			 * @brief
			 *     Constructs a new meta information object for a class type.
			 *
			 * @param decl
			 *     declaration of the type
			 *
			 * @param builtin
			 *     whether the type is a built-in type
			 *
			 */
			constexpr basic_type_info(const ast::class_declaration& decl, bool builtin)
					: _cat{builtin ? category::builtin_class : category::user_defined}
					, _declaration{&decl} {}

			/**
			 * @brief
			 *     Checks whether the type represented by this object is
			 *     built-in.
			 *
			 * @return
			 *     whether this type is built-in
			 *
			 */
			constexpr bool is_builtin() noexcept {
				return ((static_cast<std::uint8_t>(_cat) + 0u) & builtin_mask);
			}

			/**
			 * @brief
			 *     Checks whether the type represented by this object is
			 *     a primitive type.
			 *
			 * @return
			 *     whether this type is a primitive type
			 *
			 */
			constexpr bool is_primitive() noexcept {
				return ((static_cast<std::uint8_t>(_cat) + 0u) & primitive_mask);
			}

			/**
			 * @brief
			 *     Checks whether the type represented by this object is
			 *     instantiable, i.e. whether instances of this type are valid
			 *     variables.
			 *
			 * This method returns `true` for all types except `void` and
			 * `null`.
			 *
			 * @return
			 *     whether this type is instantiable
			 *
			 */
			constexpr bool is_instantiable() noexcept {
				return ((static_cast<std::uint8_t>(_cat) + 0u) & instantiable_mask);
			}

			/**
			 * @brief
			 *     Checks whether the type represented by this object is
			 *     a reference type.
			 *
			 * @return
			 *     whether this type is a reference type
			 *
			 */
			constexpr bool is_reference() noexcept {
				return !is_primitive();
			}

			/**
			 * @brief
			 *     Checks whether the type represented by this object is `null`.
			 *
			 * @return
			 *     whether this type is `null`
			 *
			 */
			constexpr bool is_null() noexcept {
				return _cat == category::null_type;
			}

			/**
			 * @brief
			 *     Checks whether the type represented by this object is `void`.
			 *
			 * @return
			 *     whether this type is `void`
			 *
			 */
			constexpr bool is_void() noexcept {
				return _cat == category::null_type;
			}

			/**
			 * @brief
			 *     Checks whether the type represented by this object is
			 *     user-defined.
			 *
			 * @return
			 *     whether this type is user-defined
			 *
			 */
			constexpr bool is_user_defined() noexcept {
				return _cat == category::user_defined;
			}

			/**
			 * @brief
			 *     Returns the declaration of this type.
			 *
			 * If this type is a primitive type or `null`, `nullptr` is
			 * returned.
			 *
			 * @return
			 *     declaration or `nullptr`
			 *
			 */
			constexpr const ast::class_declaration* declaration() noexcept {
				return _declaration;
			}

		private:

			/** @brief category of the type represented by this object */
			category _cat;

			/**
			 * @brief
			 *     declaration of the type represented by this object
			 *
			 * This pointer is `nullptr` if and only if the type represented by
			 * this object is null or a primitive type.
			 *
			 */
			const ast::class_declaration* const _declaration;

			/** @brief private constructor for non-class types */
			constexpr /* implicit */ basic_type_info(category cat)
					: _cat{cat}, _declaration{nullptr} {}

		public:

			/**
			 * @brief
			 *     Creates the `null` type.
			 *
			 * This method is meant for use inside the semantic module only.
			 *
			 * @return
			 *     `null` type
			 *
			 */
			static basic_type_info make_null_type()
			{
				return {category::null_type};
			};

			/**
			 * @brief
			 *     Creates the `void` type.
			 *
			 * This method is meant for use inside the semantic module only.
			 *
			 * @return
			 *     `void` type
			 *
			 */
			static basic_type_info make_void_type()
			{
				return {category::void_type};
			};

			/**
			 * @brief
			 *     Creates the `int` type.
			 *
			 * This method is meant for use inside the semantic module only.
			 *
			 * @return
			 *     `int` type
			 *
			 */
			static basic_type_info make_int_type()
			{
				return {category::int_type};
			};

			/**
			 * @brief
			 *     Creates the `boolean` type.
			 *
			 * This method is meant for use inside the semantic module only.
			 *
			 * @return
			 *     `boolean` type
			 *
			 */
			static basic_type_info make_boolean_type()
			{
				return {category::boolean_type};
			};

		};

		/**
		 * @brief
		 *     Extracts types from the given AST and adds them to the given
		 *     definitions.
		 *
		 * @param ast
		 *     AST to process
		 *
		 * @param builtin
		 *     whether the discovered types should be marked as built-in
		 *
		 * @param definitions
		 *     existing definitions
		 *
		 */
		void extract_type_info(const ast::program& ast, bool builtin,
							   type_definitions& definitions);
	}
}
