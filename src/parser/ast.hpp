#pragma once

#include <cstdint>
#include <vector>
#include "symbol.hpp"

namespace minijava
{
	namespace ast {

		enum class var_type : std::uint16_t
		{
			type_int,
			type_boolean,
			type_void,
			identifier
		};

		enum class binary_expression_type : std::uint16_t {
			type_logical_or,
			type_logical_and,
			type_equality,
			type_unequality,
			type_lower,
			type_lower_equal,
			type_greater,
			type_greater_equal,
			type_add,
			type_subtract,
			type_multiplay,
			type_divide,
			type_modulo,
		};

		enum class unary_expression_type : std::uint16_t  {
			type_not,
			type_negate
		};

		class node {

		public:
			/**
			 * @brief
			 *     `return`s the line number where the operation was found.
			 *
			 * If the value is 0, the line number is unknown.
			 *
			 * @returns
			 *     line number
			 *
			 */
			std::size_t line() const noexcept;

			/**
			 * @brief
			 *     `return`s the column number where the operation was found.
			 *
			 * If the value is 0, the column number is unknown.
			 *
			 * @returns
			 *     column number
			 *
			 */
			std::size_t column() const noexcept;

			/**
			 * @brief
			 *     Associates a line number with the operation.
			 *
			 * @param line
			 *     line number
			 *
			 */
			void set_line(std::size_t line) noexcept;

			/**
			 * @brief
			 *     Associates a column number with the operation.
			 *
			 * @param column
			 *     column number
			 *
			 */
			void set_column(std::size_t column) noexcept;

		private:

			/** @brief Line number where the operation was found. */
			std::size_t _line {};

			/** @brief Column number where the operation was found. */
			std::size_t _column {};
		};

		class type : public node
		{
		private:
			// type of the var
			var_type type;

			// if the var is an identifier, then this contains the name
			symbol identifier;

			// is the type an array?
			bool is_array;

			// contains the array dimension (is_array = true)
			size_t array_dimension;
		};

		class parameter : public node
		{
		private:
			type type;

			symbol name;
		};

		// interface
		class block_statement : public node {};

		// interface
		class statement : public block_statement {};

		class block : public statement
		{
		private:
			std::vector<int> numbers;
			std::vector<block_statement> block_statement;
		};

		class expression : public node
		{};

		class assignment_expression : public expression
		{
		private:
			// left side of the assignment
			expression lhs;

			// right side of the assignment
			expression rhs;
		};

		class binary_expression : public expression
		{
		private:
			// the operation to execute
			binary_expression_type type;

			// left side of the operation
			expression lhs;

			// right side of the operation
			expression rhs;
		};

		class unary_expression : public expression
		{
		private:
			// type of the operation
			unary_expression_type type;

			// expression to apply the operation
			expression expression;
		};

		// a local variable declaration (inside of blocks)
		class local_variable_statement : public block_statement
		{
		private:
			// var declaration
			type type;

			// var name
			symbol name;

			// optinal expression for assignment
			expression expression;
		};

		// a field declaration
		class field : public node
		{
		private:
			// field type
			type type;

			// name of the field
			symbol identifier;
		};

		// a method declaration
		class method : public node
		{
		private:
			// is main method?
			bool is_main;

			// parameters
			std::vector<parameter> parameters;

			// return type
			type return_type;

			// method body
			block body;
		};

		class expression_statement : public statement
		{
		private:
			expression expression;
		};

		// control statements
		class if_statement : public statement
		{
		private:
			// then block
			block then_block;

			// else block
			block else_block;

			// condition
			expression expression;
		};

		// a while statement
		class while_statement : public statement
		{
		private:
			// while condition
			expression expression;

			// block to execute
			block block;
		};

		// an return statement
		class return_statement : public statement
		{
		private:
			// optional
			expression expression;
		};

		// just an semicolon
		class empty_statement : public statement
		{};

		// a class declaration
		class class_declaration : public node
		{
		private:
			// declared fields
			std::vector<field> fields;

			// declared methods
			std::vector<method> methods;
		};

		// root node of the ast; contains class declarations
		class program
		{
		private:
			std::vector<class_declaration> classes;
		};

		// a primary expression
		class primary : public expression {
		private:
		};

		// an atomic value (bool, int, null)
		class atom : public primary
		{ };

		// boolean constant
		class boolean_literal : public atom
		{
		private:
			bool value;
		};

		// integer constant
		class integer_literal : public atom
		{
		private:
			// string representation of the int value
			symbol data;

			// int value, calculated out of data
			std::int32_t value;
		};

		// null constant
		class null_literal : public atom {};

		class method_invocation : public primary
		{
		private:
			symbol method_name;
			std::vector<expression> arguments;

			// parent node -> reference object or null, if no ref is given
			// ref.method_name(...) | method_name(...)
			// symbol ref
		};

		// done
		class ref : public primary
		{
		private:
			symbol identifier;
		};

		// done
		class new_array_object : public primary
		{
		private:
			// size of the first dimension
			expression expression;

			// array type
			var_type type;

			// number of brakets after expression IDENT[expression]([])*
			size_t array_dimension;
		};

		// an object instantiation
		class new_object : public primary
		{
		private:
			symbol class_name;
		};

		class array_access : public expression
		{
		private:
			// parent node
			// primary array;

			expression expression;
		};

		class field_access : public expression
		{
		private:
			// parent node
			// primary object;

			// name of the field
			symbol field_name;
		};

	}
}