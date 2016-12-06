/**
 * @file type_info.hpp
 *
 * @brief
 *     Methods and data structures for extracting type information from the AST.
 *
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <unordered_map>

#include <boost/functional/hash.hpp>

#include "parser/ast.hpp"
#include "symbol/symbol.hpp"


namespace minijava
{

	namespace sem
	{

		/**
		 * @brief
		 *     Meta information describing a basic, i.e. non-array, type.
		 *
		 */
		class basic_type_info
		{

		private:

			/** @brief Mask indicating that the type is built-in. */
			static constexpr std::uint8_t builtin_mask{1 << 0};

			/** @brief Mask indicating that the type is primitive. */
			static constexpr std::uint8_t primitive_mask{1 << 1};

			/** @brief Mask indicating that the type is instantiable. */
			static constexpr std::uint8_t instantiable_mask{1 << 2};

			/** @brief Type categories. */
			enum class category : std::uint8_t
			{
				user_defined  = instantiable_mask,
				builtin_class = builtin_mask | instantiable_mask,
				null_type     = builtin_mask,
				void_type     = builtin_mask | primitive_mask,
				int_type      = builtin_mask | primitive_mask | instantiable_mask,
				boolean_type  = builtin_mask | primitive_mask | instantiable_mask | (1 << 7)
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
			constexpr bool is_builtin() const noexcept
			{
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
			constexpr bool is_primitive() const noexcept
			{
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
			constexpr bool is_instantiable() const noexcept
			{
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
			constexpr bool is_reference() const noexcept
			{
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
			constexpr bool is_null() const noexcept
			{
				return (_cat == category::null_type);
			}

			/**
			 * @brief
			 *     Checks whether the type represented by this object is `void`.
			 *
			 * @return
			 *     whether this type is `void`
			 *
			 */
			constexpr bool is_void() const noexcept
			{
				return (_cat == category::void_type);
			}

			/**
			 * @brief
			 *     Checks whether the type represented by this object is `int`.
			 *
			 * @return
			 *     whether this type is `int`
			 *
			 */
			constexpr bool is_int() const noexcept
			{
				return (_cat == category::int_type);
			}

			/**
			 * @brief
			 *     Checks whether the type represented by this object is
			 *     `boolean`.
			 *
			 * @return
			 *     whether this type is `boolean`
			 *
			 */
			constexpr bool is_boolean() const noexcept
			{
				return (_cat == category::boolean_type);
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
			constexpr bool is_user_defined() const noexcept
			{
				return (_cat == category::user_defined);
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
			constexpr const ast::class_declaration* declaration() const noexcept
			{
				return _declaration;
			}

			/**
			 * @brief
			 *     Updates a hash state with the internal state of this object.
			 *
			 * @param state
			 *     hash state to update
			 *
			 */
			void append_hash(std::size_t& state) const noexcept
			{
				boost::hash_combine(state, _cat);
				boost::hash_combine(state, _declaration);
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
			const ast::class_declaration* _declaration;

			/**
			 * @brief
			 *     Private constructor for non-class types.
			 *
			 * @param cat
			 *     category of the type
			 *
			 */
			constexpr /* implicit */ basic_type_info(category cat)
					: _cat{cat}, _declaration{nullptr}
			{
			}

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
			static constexpr basic_type_info make_null_type()
			{
				return {category::null_type};
			}

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
			static constexpr basic_type_info make_void_type()
			{
				return {category::void_type};
			}

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
			static constexpr basic_type_info make_int_type()
			{
				return {category::int_type};
			}

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
			static constexpr basic_type_info make_boolean_type()
			{
				return {category::boolean_type};
			}

			/**
			 * @brief
			 *     Compares two basic types.
			 *
			 * Two `basic_type_info` instances are equal if and only if they
			 * both have the same category and same point of definition.
			 *
			 * @param lhs
			 *     first basic type to compare
			 *
			 * @param rhs
			 *     second basic type to compare
			 *
			 * @returns
			 *     whether `lhs` and `rhs` are the same type
			 *
			 */
			static constexpr bool equal(basic_type_info lhs, basic_type_info rhs) noexcept
			{
				return (lhs._cat == rhs._cat) && (lhs._declaration == rhs._declaration);
			}

		};  // class basic_type_info


		/**
		 * @brief
		 *     Compares two basic types for equality.
		 *
		 * @param lhs
		 *     first basic type to compare
		 *
		 * @param rhs
		 *     second basic type to compare
		 *
		 * @returns
		 *     `basic_type_info::equal(lhs, rhs)`
		 *
		 */
		constexpr bool operator==(basic_type_info lhs, basic_type_info rhs) noexcept
		{
			return basic_type_info::equal(lhs, rhs);
		}


		/**
		 * @brief
		 *     Compares two basic types for inequality.
		 *
		 * @param lhs
		 *     first basic type to compare
		 *
		 * @param rhs
		 *     second basic type to compare
		 *
		 * @returns
		 *     `! basic_type_info::equal(lhs, rhs)`
		 *
		 */
		constexpr bool operator!=(basic_type_info lhs, basic_type_info rhs) noexcept
		{
			return !basic_type_info::equal(lhs, rhs);
		}


		/**
		 * @brief
		 *     Inserts a human-readable representation of the basic type into
		 *     the stream.
		 *
		 * The generated output may not be valid MiniJava.
		 *
		 * @param os
		 *     stream to write to
		 *
		 * @param bti
		 *     `basic_type_info` to write
		 *
		 * @returns
		 *     reference to `os`
		 *
		 */
		std::ostream& operator<<(std::ostream& os, const basic_type_info bti);


		/** @brief Mapping from class names to their types. */
		using class_definitions = std::unordered_map<symbol, basic_type_info>;


		/**
		 * @brief
		 *     Extracts classes from `ast` and adds them to `definitions`.
		 *
		 * @param ast
		 *     AST to process
		 *
		 * @param builtin
		 *     whether the discovered types should be marked as builtins
		 *
		 * @param definitions
		 *     data structure to populate with the results
		 *
		 * @throws semantic_error
		 *     if a duplicate class name is found
		 *
		 */
		void extract_type_info(const ast::program& ast, bool builtin,
		                       class_definitions& definitions);


	}  // namespace sem

}  // namespace minijava


namespace std
{

	/**
	 * @brief
	 *     `std::hash` specialization for `basic_type_info` objects.
	 *
	 */
	template <>
	struct hash<minijava::sem::basic_type_info>
	{
		/**
		 * @brief
		 *     Hashes the given `basic_type_info`.
		 *
		 * @param bti
		 *     `basic_type_info` to hash
		 *
		 * @returns
		 *     hash value
		 *
		 */
		size_t operator()(const minijava::sem::basic_type_info& bti) const noexcept
		{
			auto seed = size_t{};
			bti.append_hash(seed);
			return seed;
		}
	};

}  // namespace std
