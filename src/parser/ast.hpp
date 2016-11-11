/**
 * @file ast.hpp
 *
 * @brief
 *     Defines AST data structure.
 *
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <boost/variant.hpp>

#include "symbol.hpp"

namespace minijava
{
	namespace ast
	{
		/**
		 * Binary operations supported by MiniJava
		 */
		enum class binary_operation_type : std::uint16_t
		{
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

		/**
		 * Unary operations supported by MiniJava
		 */
		enum class unary_operation_type : std::uint8_t
		{
			type_not,
			type_negate
		};

		/**
		 * Primitive types in MiniJava
		 */
		enum class primitive_type : std::uint8_t
		{
			type_int,
			type_boolean,
			type_void
		};


		/**
		 * Base class for AST nodes
		 */
		class node
		{

		public:

			/**
			 * @brief
			 *     `default`ed default constructor.
			 */
			node() = default;

			/**
			 * @brief
			 *     `delete`d copy constructor.
			 *
			 * `node` objects are not copyable.
			 *
			 * @param other
			 *     *N/A*
			 *
			 */
			node(const node& other) = delete;

			/**
			 * @brief
			 *     `delete`d copy-assignment operator.
			 *
			 * `node` objects are not copyable or movable.
			 *
			 * @param other
			 *     *N/A*
			 *
			 * @returns
			 *     *N/A*
			 *
			 */
			node& operator=(const node& other) = delete;

			/**
			 * @brief
			 *     `default`ed virtual destructor.
			 */
			virtual ~node() = default;

			/**
			 * @brief
			 *     `return`s the line number where the node was found.
			 *
			 * If the value is 0, the line number is unknown.
			 *
			 * @returns
			 *     line number
			 *
			 */
			std::size_t line() const noexcept {
				return _line;
			}

			/**
			 * @brief
			 *     `return`s the column number where the node was found.
			 *
			 * If the value is 0, the column number is unknown.
			 *
			 * @returns
			 *     column number
			 *
			 */
			std::size_t column() const noexcept {
				return _column;
			}

			/**
			 * @brief
			 *     Associates a line number with the node.
			 *
			 * @param line
			 *     line number
			 *
			 */
			void set_line(std::size_t line) noexcept {
				_line = line;
			}

			/**
			 * @brief
			 *     Associates a column number with the node.
			 *
			 * @param column
			 *     column number
			 *
			 */
			void set_column(std::size_t column) noexcept {
				_column = column;
			}

		private:

			/** @brief Line number where the operation was found. */
			std::size_t _line {};

			/** @brief Column number where the operation was found. */
			std::size_t _column {};
		};


		/**
		 * Convenience typedef for a type representing an arbitrary MiniJava
		 * type (either primitive or user-defined).
		 */
		using type_name = boost::variant<primitive_type, symbol>;

		/**
		 * Type name AST node
		 */
		class type final : public node
		{

		public:

			/**
			 * @brief
			 *     Constructs a type node representing a built-in type.
			 *
			 * @param type
			 *     variable type
			 *
			 * @param rank
			 *     rank of the array or 0 if this type is not an array type
			 */
			type(primitive_type type, std::size_t rank)
					: node{}, _type{type}, _rank{rank} {}

			/**
			 * @brief
			 *     Constructs a type node representing a user-defined type.
			 *
			 * @param type
			 *     symbol representing the type name
			 *
			 * @param rank
			 *     rank of the array or 0 if this type is not an array type
			 */
			type(symbol type, std::size_t rank)
					: node{}, _type{type}, _rank{rank}
			{
				assert(!_type.empty());
			}

			/**
			 * @brief
			 *     returns the rank of the array
			 *
			 * If this type is not an array type, `0` is returned.
			 *
			 * @return array rank or 0 if this type is not an array type
			 */
			std::size_t rank() const noexcept
			{
				return _rank;
			}

		private:

			/** @brief user-defined or built-in type represented by this node */
			type_name _type;

			/** @brief rank of the array or 0 if this node does not represent an array type */
			std::size_t _rank;
		};

		/**
		 * Variable declaration AST node
		 */
		class var_decl final : public node
		{

		public:

			/**
			 * @brief
			 *     Constructs a variable declaration node.
			 *
			 * @param type
			 *     variable type
			 *
			 * @param name
			 *     variable name
			 */
			var_decl(std::unique_ptr<type> type, symbol name)
					: _type{std::move(type)}, _name{name}
			{
				assert(_type);
				assert(!_name.empty());
			}

		private:

			/** @brief type of the variable */
			std::unique_ptr<type> _type;

			/** @brief symbol representing the variable name */
			symbol _name;
		};


		// region expressions

		/**
		 * Base class for expression AST nodes
		 */
		class expression : public node {};

		/**
		 * Assignment expression AST node
		 */
		class assignment_expression final : public expression
		{

		public:

			/**
			 * @brief
			 *     Constructs an assignment expression node.
			 *
			 * @param lhs
			 *     expression on the left side of the assignment
			 *
			 * @param rhs
			 *     expression on the right side of the assignment
			 */
			assignment_expression(std::unique_ptr<expression> lhs,
			                      std::unique_ptr<expression> rhs)
					: _lhs{std::move(lhs)}, _rhs{std::move(rhs)}
			{
				assert(_lhs);
				assert(_rhs);
			}

		private:

			/** @brief expression on the left side of the assignment */
			std::unique_ptr<expression> _lhs;

			/** @brief expression on the right side of the assignment */
			std::unique_ptr<expression> _rhs;
		};

		/**
		 * Binary expression AST node
		 */
		class binary_expression final : public expression
		{

		public:

			/**
			 * @brief
			 *     Constructs a binary expression node.
			 *
			 * @param type
			 *     binary operator
			 *
			 * @param lhs
			 *     expression on the left side of the operator
			 *
			 * @param rhs
			 *     expression on the right side of the operator
			 *
			 */
			binary_expression(binary_operation_type type,
			                  std::unique_ptr<expression> lhs,
			                  std::unique_ptr<expression> rhs)
					: _type{type}, _lhs{std::move(lhs)}, _rhs{std::move(rhs)}
			{
				assert(_lhs);
				assert(_rhs);
			}

		private:

			/** @brief type of the binary operation */
			binary_operation_type _type;

			/** @brief expression on the left side of the operator */
			std::unique_ptr<expression> _lhs;

			/** @brief expression on the right side of the operator */
			std::unique_ptr<expression> _rhs;
		};

		/**
		 * Unary expression AST node
		 */
		class unary_expression final : public expression
		{

		public:

			/**
			 * @brief
			 *     Constructs an unary expression node.
			 *
			 * @param type
			 *     unary operator
			 *
			 * @param expression
			 *     expression to which the operation is applied
			 *
			 */
			unary_expression(unary_operation_type type,
			                 std::unique_ptr<expression> expression)
					: _type{type}, _expression{std::move(expression)}
			{
				assert(_expression);
			}

		private:

			/** @brief type of the unary operation */
			unary_operation_type _type;

			/** @brief expression to which the unary operation is applied */
			std::unique_ptr<expression> _expression;
		};

		/**
		 * Object instantiation AST node
		 */
		class object_instantiation final : public expression
		{

		public:

			/**
			 * @brief
			 *     Constructs an object instantiation node.
			 *
			 * @param class_name
			 *     class which is being instantiated
			 */
			object_instantiation(symbol class_name) : _class_name{class_name}
			{
				assert(!_class_name.empty());
			}

		private:

			/** @brief name of the class being instantiated */
			symbol _class_name;

		};

		/**
		 * Array instantiation AST noded
		 */
		class array_instantiation final : public expression
		{

		public:

			/**
			 * @brief
			 *     Constructs an array instantiation node
			 *
			 * @param type
			 *     type to instantiate
			 *
			 * @param extent
			 *     expression evaluating to the extent of the first dimension
			 *
			 */
			array_instantiation(std::unique_ptr<type> type,
								std::unique_ptr<expression> extent)
					: _type{std::move(type)}, _extent{std::move(extent)}
			{
				assert(_type);
				assert(_type->rank() > 0);
				assert(_extent);
			}

		private:

			/** @brief type to instantiate */
			std::unique_ptr<type> _type;

			/**
			 * @brief
			 *     expression evaluating to the extent of the first dimension
			 *
			 */
			std::unique_ptr<expression> _extent;
		};

		/**
		 * Array access AST node
		 */
		class array_access final : public expression
		{

		public:

			/**
			 * @brief
			 *     Constructs an array access node.
			 *
			 * @param target
			 *     target array to access
			 *
			 * @param index
			 *     index to access
			 */
			array_access(std::unique_ptr<expression> target,
						 std::unique_ptr<expression> index)
					: _target{std::move(target)}, _index{std::move(index)}
			{
				assert(_target);
				assert(_index);
			}

		private:

			/** @brief target array to access */
			std::unique_ptr<expression> _target;

			/** @brief index to access */
			std::unique_ptr<expression> _index;
		};

		/**
		 * @brief
		 *     Variable access AST node.
		 *
		 * Represents a reference to a field or local variable.
		 *
		 */
		class variable_access final : public expression
		{

		public:

			/**
			 * @brief
			 *     constructs a variable access node
			 *
			 * @param target
			 *     target object to access or null pointer
			 *
			 * @param name
			 *     variable or field to access
			 *
			 */
			variable_access(std::unique_ptr<expression> target, symbol name)
					: _target{std::move(target)}, _name{name}
			{
				assert(!_name.empty());
			}

		private:

			/** @brief target object to access or null pointer */
			std::unique_ptr<expression> _target;

			/** @brief variable or field to access */
			symbol _name;
		};

		/**
		 * Method invocation AST node
		 */
		class method_invocation final : public expression
		{

		public:

			/**
			 * @brief
			 *     constructs a method invocation node
			 *
			 * @param target
			 *     target object to access or null pointer
			 *
			 * @param name
			 *     method to invoke
			 *
			 */
			method_invocation(std::unique_ptr<expression> target, symbol name)
					: _target{std::move(target)}, _name{std::move(name)}
			{
				assert(!_name.empty());
			}

			/**
			 * Adds an argument to the method invocation.
			 *
			 * @param arg argument
			 */
			void add_argument(std::unique_ptr<expression> arg)
			{
				assert(arg);
				_arguments.push_back(std::move(arg));
			}

		private:

			/** @brief target object to access or null pointer */
			std::unique_ptr<expression> _target;

			/** @brief method to invoke */
			symbol _name;

			/** @brief method arguments */
			std::vector<std::unique_ptr<expression>> _arguments {};
		};

		/**
		 * AST node for a reference to `this`
		 */
		class this_ref final : public expression {};

		/**
		 * Base class for AST nodes representing constants
		 */
		class constant : public expression {};

		/**
		 * Boolean constant AST node
		 */
		class boolean_constant final : public constant
		{

		public:

			/**
			 * Constructs a boolean constant node.
			 *
			 * @param value
			 *     value of the constant
			 *
			 */
			boolean_constant(bool value) : _value{value} {}

		private:

			/** @brief value of the constant */
			bool _value;
		};

		/**
		 * Integer constant AST node
		 */
		class integer_constant final : public constant
		{

		public:

			/**
			 * Constructs an integer constant AST node.
			 *
			 * @param literal
			 *     integer literal specifying the value of the constant
			 *
			 */
			integer_constant(symbol literal) : _literal{literal}
			{
				assert(!literal.empty());
			}

		private:

			/** @brief integer literal specifying the value of the constant */
			symbol _literal;
		};

		/**
		 * Null constant AST node
		 */
		class null_constant final : public constant {};

		// endregion


		// region statements

		/**
		 * @brief
		 *     Base class for block statement AST nodes.
		 *
		 * Block statements are either statements or variable declarations.
		 */
		class block_statement : public node {};

		/**
		 * Local variable declaration AST node
		 */
		class local_variable_statement final : public block_statement
		{

		public:

			/**
			 * @brief
			 *     Constructs a local variable declaration node.
			 *
			 * @param declaration
			 *     variable declaration
			 *
			 * @param initial_value
			 *     initial value to assign to the variable or null pointer
			 *
			 */
			local_variable_statement(std::unique_ptr<var_decl> declaration,
									 std::unique_ptr<expression> initial_value)
					: _declaration{std::move(declaration)}
					, _initial_value{std::move(initial_value)}
			{
				assert(_declaration);
			}

		private:

			/** @brief variable declaration */
			std::unique_ptr<var_decl> _declaration;

			/** @brief inital value or null pointer if the variable is not initialized immediately */
			std::unique_ptr<expression> _initial_value;
		};

		/**
		 * @brief
		 *     Base classes for statement AST nodes.
		 *
		 * Local variable declarations are not regular statements.
		 */
		class statement : public block_statement {};

		/**
		 * Expression statement AST node
		 */
		class expression_statement final : public statement
		{
			
		public:

			/**
			 * @brief
			 *     Constructs an expression statement node.
			 * 
			 * @param expression
			 *     expression to interpret as a statement
			 *     
			 */
			expression_statement(std::unique_ptr<expression> expression)
					: _expression{std::move(expression)}
			{
				assert(_expression);
			}

		private:
			
			/** @brief expression to interpret as a statement */
			std::unique_ptr<expression> _expression;
		};

		/**
		 * Block AST node
		 */
		class block final : public statement
		{

		public:

			/**
			 * Adds a block statement to the end of this block.
			 *
			 * @param stmt block statement to add
			 */
			void add_block_statement(std::unique_ptr<block_statement> stmt)
			{
				assert(stmt);
				_body.push_back(std::move(stmt));
			}

		private:

			/** @brief statements contained in this block */
			std::vector<std::unique_ptr<block_statement>> _body {};
		};

		/**
		 * If statement AST node
		 */
		class if_statement final : public statement
		{

		public:

			/**
			 * Constructs a new if statement.
			 *
			 * @param then_stmt
			 *     statement to execute if the condition evaluates to `true`
			 *
			 * @param else_stmt
			 *     statement to execute if the condition evaluates to `false`
			 *     or null if no such statement exists
			 *
			 * @param condition
			 *     branch condition
			 *
			 */
			if_statement(std::unique_ptr<statement> then_stmt,
						 std::unique_ptr<statement> else_stmt,
						 std::unique_ptr<expression> condition)
					: _condition{std::move(condition)}
					, _then_branch{std::move(then_stmt)}
					, _else_branch{std::move(else_stmt)}
			{
				assert(_condition);
				assert(_then_branch);
			}

		private:

			/** @brief branch condition */
			std::unique_ptr<expression> _condition;

			/** @brief statement to execute if the condition evaluates to `true` */
			std::unique_ptr<statement> _then_branch;

			/** @brief statement to execute if the condition evaluates to `false` or null if no such statement exists */
			std::unique_ptr<statement> _else_branch;
		};

		/**
		 * While statement AST node
		 */
		class while_statement final : public statement
		{

		public:

			/**
			 * Constructs a while statement
			 *
			 * @param condition
			 *     loop condition
			 *
			 * @param body
			 *     loop body
			 *
			 */
			while_statement(std::unique_ptr<expression> condition,
							std::unique_ptr<statement> body)
					: _condition{std::move(condition)}, _body{std::move(body)}
			{
				assert(_condition);
				assert(_body);
			}

		private:

			/** @brief loop condition */
			std::unique_ptr<expression> _condition;

			/** @brief loop body */
			std::unique_ptr<statement> _body;
		};

		/**
		 * Return statement AST node
		 */
		class return_statement final : public statement
		{

		public:

			/**
			 * @brief
			 *     Constructs a return statement node.
			 *
			 * @param value
			 *     return value or null pointer if void is returned
			 *
			 */
			return_statement(std::unique_ptr<expression> value)
					: _value{std::move(value)} {}

		private:

			/** @brief return value or null pointer if void is returned */
			std::unique_ptr<expression> _value;
		};

		// endregion


		// region structural elements

		/**
		 * Main method AST node
		 */
		class main_method final : public node
		{

		public:

			/**
			 * @brief
			 *     Constructs a main method node.
			 *
			 * @param name
			 *     method name
			 *
			 * @param body
			 *     method body
			 *
			 */
			main_method(symbol name, std::unique_ptr<block> body)
					: _name{name}, _body{std::move(body)}
			{
				assert(!_name.empty());
				assert(_body);
			}

		private:

			/** @brief method name */
			symbol _name;

			/** @brief method body */
			std::unique_ptr<block> _body;
		};

		/**
		 * Method AST node
		 */
		class method final : public node
		{

		public:

			/**
			 * Constructs a method node.
			 *
			 * @param name
			 *     method name
			 *
			 * @param return_type
			 *     method return type
			 *
			 * @param body
			 *     method body
			 *
			 */
			method(symbol name, std::unique_ptr<type> return_type,
				   std::unique_ptr<block> body)
					: _name{name}, _return_type{std::move(return_type)},
					  _body{std::move(body)}
			{

			}

			/**
			 * Adds a parameter declaration to this method.
			 *
			 * @param param
			 *     parameter declaration
			 *
			 */
			void add_parameter(std::unique_ptr<var_decl> param)
			{
				assert(param);
				_parameters.push_back(std::move(param));
			}

		private:

			/** @brief method name */
			symbol _name;

			/** @brief method return type */
			std::unique_ptr<type> _return_type;

			/** @brief method body */
			std::unique_ptr<block> _body;

			/** @brief method parameters */
			std::vector<std::unique_ptr<var_decl>> _parameters {};
		};

		/**
		 * Class declaration AST node
		 */
		class class_declaration final : public node
		{

		public:

			/**
			 * Adds a field declaration to this class.
			 *
			 * @param field
			 *     field declaration
			 *
			 */
			void add_field(std::unique_ptr<var_decl> field)
			{
				assert(field);
				_fields.push_back(std::move(field));
			}

			/**
			 * Adds a method declaration to this class.
			 *
			 * @param method
			 *     method declaration
			 *
			 */
			void add_method(std::unique_ptr<method> method)
			{
				assert(method);
				_methods.push_back(std::move(method));
			}

			/**
			 * Adds a main method declaration to this class.
			 *
			 * @param main_method
			 *     main method declaration
			 *
			 */
			void add_main_method(std::unique_ptr<main_method> main_method)
			{
				assert(main_method);
				_main_methods.push_back(std::move(main_method));
			}

		private:

			/** @brief declared fields */
			std::vector<std::unique_ptr<var_decl>> _fields {};

			/** @brief declared methods */
			std::vector<std::unique_ptr<method>> _methods {};

			/** @brief declared main methods */
			std::vector<std::unique_ptr<main_method>> _main_methods {};
		};

		/**
		 * Root node of the AST
		 */
		class program final : public node
		{

		public:

			/**
			 * Adds a class declaration to this program.
			 *
			 * @param class_decl
			 *     class declaration
			 *
			 */
			void add_class(std::unique_ptr<class_declaration> class_decl)
			{
				assert(class_decl);
				_classes.push_back(std::move(class_decl));
			}

		private:

			/** @brief classes declared in this program */
			std::vector<std::unique_ptr<class_declaration>> _classes {};
		};

		// endregion


		// FIXME: add visitor base class: public methods (final classes) which
		// call private methods by default (base node classes)
	}
}
