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
#include <utility>
#include <vector>

#include <boost/variant.hpp>

#include "symbol/symbol.hpp"

namespace minijava
{
	namespace ast
	{
		class node;
		class type;
		class var_decl;
		class expression;
		class binary_expression;
		class unary_expression;
		class object_instantiation;
		class array_instantiation;
		class array_access;
		class variable_access;
		class method_invocation;
		class this_ref;
		class constant;
		class boolean_constant;
		class integer_constant;
		class null_constant;
		class block_statement;
		class local_variable_statement;
		class statement;
		class expression_statement;
		class block;
		class if_statement;
		class while_statement;
		class return_statement;
		class empty_statement;
		class main_method;
		class method;
		class class_declaration;
		class program;

		/**
		 * @brief
		 *     Base class for AST visitors
		 */
		class visitor
		{
		protected:
			/**
			 * @brief
			 *     Visits a generic AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit_node(const node& node);

			/**
			 * @brief
			 *     Visits an expression AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit_expression(const expression& node);

			/**
			 * @brief
			 *     Visits a constant AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit_constant(const constant& node);

			/**
			 * @brief
			 *     Visits a block_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit_block_statement(const block_statement& node);

			/**
			 * @brief
			 *     Visits a statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit_statement(const statement& node);

		public:
			/**
			 * @brief
			 *     Visits a type AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const type& node);

			/**
			 * @brief
			 *     Visits a var_decl AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const var_decl& node);

			/**
			 * @brief
			 *     Visits a binary_expression AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const binary_expression& node);

			/**
			 * @brief
			 *     Visits a unary_expression AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const unary_expression& node);

			/**
			 * @brief
			 *     Visits an object_instantiation AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const object_instantiation& node);

			/**
			 * @brief
			 *     Visits an array_instantiation AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const array_instantiation& node);

			/**
			 * @brief
			 *     Visits an array_access AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const array_access& node);

			/**
			 * @brief
			 *     Visits a variable_access AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const variable_access& node);

			/**
			 * @brief
			 *     Visits a method_invocation AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const method_invocation& node);

			/**
			 * @brief
			 *     Visits a this_ref AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const this_ref& node);

			/**
			 * @brief
			 *     Visits a boolean_constant AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const boolean_constant& node);

			/**
			 * @brief
			 *     Visits an integer_constant AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const integer_constant& node);

			/**
			 * @brief
			 *     Visits a null_constant AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const null_constant& node);

			/**
			 * @brief
			 *     Visits a local_variable_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const local_variable_statement& node);

			/**
			 * @brief
			 *     Visits an expression_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const expression_statement& node);

			/**
			 * @brief
			 *     Visits a block AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const block& node);

			/**
			 * @brief
			 *     Visits an if_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const if_statement& node);

			/**
			 * @brief
			 *     Visits a while_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const while_statement& node);

			/**
			 * @brief
			 *     Visits a return_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const return_statement& node);

			/**
			 * @brief
			 *     Visits an empty_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const empty_statement& node);

			/**
			 * @brief
			 *     Visits a main_method AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const main_method& node);

			/**
			 * @brief
			 *     Visits a method AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const method& node);

			/**
			 * @brief
			 *     Visits a class_declaration AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const class_declaration& node);

			/**
			 * @brief
			 *     Visits a program AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const program& node);

		};

		/**
		 * Binary operations supported by MiniJava
		 */
		enum class binary_operation_type : std::uint16_t
		{
			assign,
			logical_or,
			logical_and,
			equal,
			not_equal,
			less_than,
			less_equal,
			greater_than,
			greater_equal,
			plus,
			minus,
			multiply,
			divide,
			modulo,
		};

		/**
		 * Unary operations supported by MiniJava
		 */
		enum class unary_operation_type : std::uint8_t
		{
			logical_not,
			minus
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
		 * @brief
		 *     Base class for AST nodes
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

			/**
			 * @brief
			 *     Accepts a visitor
			 *
			 * @param v
			 *     The visitor
			 */
			virtual void accept(visitor& v) const = 0;

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
		 * @brief
		 *     Type name AST node
		 */
		class type final : public node
		{

		public:

			/**
			 * @brief
			 *     Constructs a type node representing a built-in type.
			 *
			 * @param name
			 *     primitive type name
			 *
			 * @param rank
			 *     rank of the array or 0 if this type is not an array type
			 *
			 */
			type(primitive_type name, std::size_t rank = 0)
					: node{}, _name{name}, _rank{rank} {}

			/**
			 * @brief
			 *     Constructs a type node representing a user-defined type.
			 *
			 * @param name
			 *     symbol representing the type name
			 *
			 * @param rank
			 *     rank of the array or 0 if this type is not an array type
			 *
			 */
			type(symbol name, std::size_t rank = 0)
					: node{}, _name{name}, _rank{rank}
			{
				assert(!_name.empty());
			}

			/**
			 * @brief
			 *     returns the type name
			 *
			 * @return
			 *     primitive type or symbol representing the name of the
			 *     user-defined type
			 *
			 */
			const type_name& name() const noexcept
			{
				return _name;
			}

			/**
			 * @brief
			 *     returns the rank of the array
			 *
			 * If this type is not an array type, `0` is returned.
			 *
			 * @return
			 *     array rank or 0 if this type is not an array type
			 *
			 */
			std::size_t rank() const noexcept
			{
				return _rank;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief user-defined or built-in type represented by this node */
			type_name _name;

			/** @brief rank of the array or 0 if this node does not represent an array type */
			std::size_t _rank;
		};

		/**
		 * @brief
		 *     Variable declaration AST node
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
			 *
			 */
			var_decl(std::unique_ptr<type> type, symbol name)
					: _type{std::move(type)}, _name{name}
			{
				assert(_type);
				assert(!_name.empty());
			}

			/**
			 * @brief
			 *     Returns the type of this variable.
			 *
			 * @return
			 *     type of this variable
			 *
			 */
			const type& var_type() const noexcept
			{
				return *_type;
			}

			/**
			 * @brief
			 *     Returns the name of this variable.
			 *
			 * @return
			 *     name of this variable
			 *
			 */
			symbol name() const noexcept
			{
				return _name;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief type of this variable */
			std::unique_ptr<type> _type;

			/** @brief symbol representing the name of this variable */
			symbol _name;
		};


		// region expressions

		/**
		 * @brief
		 *     Base class for expression AST nodes
		 */
		class expression : public node {};

		/**
		 * @brief
		 *     Binary expression AST node
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

			/**
			 * @brief
			 *     Returns the type of this operation
			 *
			 * @return
			 *     type of this operation
			 *
			 */
			binary_operation_type type() const noexcept
			{
				return _type;
			}

			/**
			 * @brief
			 *     Returns the expression on the left side of this operation
			 *
			 * @return
			 *     left side of this operation
			 *
			 */
			const expression& lhs() const noexcept
			{
				return *_lhs;
			}

			/**
			 * @brief
			 *     Returns the expression on the right side of this operation
			 *
			 * @return
			 *     right side of this operation
			 *
			 */
			const expression& rhs() const noexcept
			{
				return *_rhs;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
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
		 * @brief
		 *     Unary expression AST node
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
			 * @param target
			 *     expression to which the operation is applied
			 *
			 */
			unary_expression(unary_operation_type type,
			                 std::unique_ptr<expression> target)
					: _type{type}, _target{std::move(target)}
			{
				assert(_target);
			}

			/**
			 * @brief
			 *     Returns the type of this operation
			 *
			 * @return
			 *     type of this operation
			 *
			 */
			unary_operation_type type() const noexcept
			{
				return _type;
			}

			/**
			 * @brief
			 *     Returns the expression to which this operation applies
			 *
			 * @return
			 *     target of this operation
			 *
			 */
			const expression& target() const noexcept
			{
				return *_target;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief type of the unary operation */
			unary_operation_type _type;

			/** @brief expression to which the unary operation is applied */
			std::unique_ptr<expression> _target;
		};

		/**
		 * @brief
		 *     Object instantiation AST node
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

			/**
			 * @brief
			 *     Returns the name of the class which is bein instantiated.
			 *
			 * @return
			 *     class name
			 *
			 */
			symbol class_name() const noexcept
			{
				return _class_name;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief name of the class being instantiated */
			symbol _class_name;

		};

		/**
		 * @brief
		 *     Array instantiation expression AST node
		 */
		class array_instantiation final : public expression
		{

		public:

			/**
			 * @brief
			 *     Constructs an array instantiation node.
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

			/**
			 * @brief
			 *     Returns the type being instantiated.
			 *
			 * @return
			 *     type
			 *
			 */
			const type& array_type() const noexcept
			{
				return *_type;
			}

			/**
			 * @brief
			 *     Returns the expression evaluating to the extent of the first
			 *     dimension.
			 *
			 * @return
			 *     extent of the first dimension
			 *
			 */
			const expression& extent() const noexcept
			{
				return *_extent;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief type to instantiate */
			std::unique_ptr<type> _type;

			/** @brief expression evaluating to the extent of the first dimension */
			std::unique_ptr<expression> _extent;
		};

		/**
		 * @brief
		 *     Array access expression AST node
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

			/**
			 * @brief
			 *     Returns the target of this array access.
			 *
			 * @return
			 *     target expression
			 *
			 */
			const expression& target() const noexcept
			{
				return *_target;
			}

			/**
			 * @brief
			 *     Returns the index expression.
			 *
			 * @return
			 *     index expression
			 *
			 */
			const expression& index() const noexcept
			{
				return *_index;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief target array to access */
			std::unique_ptr<expression> _target;

			/** @brief index to access */
			std::unique_ptr<expression> _index;
		};

		/**
		 * @brief
		 *     Variable access expression AST node.
		 *
		 * Represents a reference to a field or local variable.
		 *
		 */
		class variable_access final : public expression
		{

		public:

			/**
			 * @brief
			 *     Constructs a variable access node.
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

			/**
			 * @brief
			 *     Returns the target whose member is accessed.
			 *
			 * May return `nullptr` if this node represents a local variable
			 * access or an unqualifier member access.
			 *
			 * @return
			 *     target expression or `nullptr`
			 *
			 */
			const expression* target() const noexcept
			{
				return _target.get();
			}

			/**
			 * @brief
			 *     Returns the name of the variable or field being accessed.
			 *
			 * @return
			 *     variable/field name
			 *
			 */
			symbol name() const noexcept
			{
				return _name;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief target object to access or null pointer */
			std::unique_ptr<expression> _target;

			/** @brief variable or field to access */
			symbol _name;
		};

		/**
		 * @brief
		 *     Method invocation expression AST node
		 */
		class method_invocation final : public expression
		{

		public:

			/**
			 * @brief
			 *     Constructs a method invocation node.
			 *
			 * @param target
			 *     target object to access or null pointer
			 *
			 * @param name
			 *     method to invoke
			 *
			 * @param arguments
			 *     arguments passed to the method
			 *
			 */
			method_invocation(std::unique_ptr<expression> target, symbol name,
							  std::vector<std::unique_ptr<expression>> arguments)
					: _target{std::move(target)}, _name{name}
					, _arguments{std::move(arguments)}
			{
				assert(!_name.empty());
				assert(std::all_of(_arguments.begin(), _arguments.end(), [](auto&& el){ return !!el; }));
			}

			/**
			 * @brief
			 *     Returns the target whose method is called.
			 *
			 * May return `nullptr` if the method name was not qualified.
			 *
			 * @return
			 *     target expression or `nullptr`
			 *
			 */
			const expression* target() const noexcept
			{
				return _target.get();
			}

			/**
			 * @brief
			 *     Returns the name of the method being called.
			 *
			 * @return
			 *     method name
			 *
			 */
			symbol name() const noexcept
			{
				return _name;
			}

			/**
			 * @brief
			 *     Returns the arguments passed to the method.
			 *
			 * @return
			 *     arguments
			 *
			 */
			const std::vector<std::unique_ptr<expression>>& arguments() const noexcept
			{
				return _arguments;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief target object to access or null pointer */
			std::unique_ptr<expression> _target;

			/** @brief method to invoke */
			symbol _name;

			/** @brief arguments passed to the method */
			std::vector<std::unique_ptr<expression>> _arguments;
		};

		/**
		 * @brief
		 *     AST node for a `this` expression
		 */
		class this_ref final : public expression
		{

		public:

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}
		};

		/**
		 * @brief
		 *     Base class for expression AST nodes representing constants
		 */
		class constant : public expression {};

		/**
		 * @brief
		 *     Boolean constant AST node
		 */
		class boolean_constant final : public constant
		{

		public:

			/**
			 * @brief
			 *     Constructs a boolean constant node.
			 *
			 * @param value
			 *     value of the constant
			 *
			 */
			boolean_constant(bool value) : _value{value} {}

			/**
			 * @brief
			 *     Returns the value of this constant.
			 *
			 * @return
			 *     value
			 *
			 */
			bool value() const noexcept
			{
				return _value;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief value of the constant */
			bool _value;
		};

		/**
		 * @brief
		 *     Integer constant AST node
		 */
		class integer_constant final : public constant
		{

		public:

			/**
			 * @brief
			 *     Constructs an integer constant AST node.
			 *
			 * @param literal
			 *     integer literal specifying the value of the constant
			 *
			 */
			integer_constant(symbol literal) : _literal{literal}
			{
				assert(!literal.empty());
			}

			/**
			 * @brief
			 *     Returns the integer literal specifying the value of this
			 *     constant.
			 *
			 * @return
			 *     integer literal
			 *
			 */
			symbol literal() const noexcept
			{
				return _literal;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief integer literal specifying the value of the constant */
			symbol _literal;
		};

		/**
		 * @brief
		 *     Null constant AST node
		 */
		class null_constant final : public constant
		{

		public:

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}
		};

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
		 * @brief
		 *     Local variable declaration AST node
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

			/**
			 * @brief
			 *     Returns the declaration of this local variable.
			 *
			 * @return
			 *     variable declaration
			 */
			const var_decl& declaration() const noexcept
			{
				return *_declaration;
			}

			/**
			 * @brief
			 *     Returns the initial value of this local variable.
			 *
			 * May return `nullptr` if the variable is not initialized
			 * immediately.
			 *
			 * @return
			 *     initialization expression or `nullptr`
			 *
			 */
			const expression* initial_value() const noexcept
			{
				return _initial_value.get();
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief variable declaration */
			std::unique_ptr<var_decl> _declaration;

			/** @brief inital value or null pointer if the variable is not initialized immediately */
			std::unique_ptr<expression> _initial_value;
		};

		/**
		 * @brief
		 *     Base class for statement AST nodes.
		 *
		 * Local variable declarations are not regular statements.
		 */
		class statement : public block_statement {};

		/**
		 * @brief
		 *     Expression statement AST node
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

			/**
			 * @brief
			 *     Returns the expression contained in this statement.
			 *
			 * @return
			 *     expression
			 *
			 */
			const expression& inner_expression() const noexcept
			{
				return *_expression;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief expression to interpret as a statement */
			std::unique_ptr<expression> _expression;
		};

		/**
		 * @brief
		 *     Block AST node
		 */
		class block final : public statement
		{

		public:

			/**
			 * @brief
			 *     Constructs a block node.
			 *
			 * @param statements
			 *     statements inside the block
			 *
			 */
			block(std::vector<std::unique_ptr<block_statement>> statements)
					: _body{std::move(statements)}
			{
				assert(std::all_of(_body.begin(), _body.end(), [](auto&& el){ return !!el; }));
			}

			/**
			 * @brief
			 *     Returns the body of this block.
			 *
			 * @return
			 *     block body
			 *
			 */
			const std::vector<std::unique_ptr<block_statement>>& body() const noexcept
			{
				return _body;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief statements contained in this block */
			std::vector<std::unique_ptr<block_statement>> _body;
		};

		/**
		 * @brief
		 *     If statement AST node
		 */
		class if_statement final : public statement
		{

		public:

			/**
			 * @brief
			 *     Constructs an if statement node.
			 *
			 * @param condition
			 *     branch condition
			 *
			 * @param then_stmt
			 *     statement to execute if the condition evaluates to `true`
			 *
			 * @param else_stmt
			 *     statement to execute if the condition evaluates to `false`
			 *     or null if no such statement exists
			 *
			 */
			if_statement(std::unique_ptr<expression> condition,
						 std::unique_ptr<statement> then_stmt,
						 std::unique_ptr<statement> else_stmt)
					: _condition{std::move(condition)}
					, _then_branch{std::move(then_stmt)}
					, _else_branch{std::move(else_stmt)}
			{
				assert(_condition);
				assert(_then_branch);
			}

			/**
			 * @brief
			 *     Returns the branch condition.
			 *
			 * @return
			 *     condition
			 *
			 */
			const expression& condition() const noexcept
			{
				return *_condition;
			}

			/**
			 * @brief
			 *     Returns the statement which is executed if the condition
			 *     evaluates to `true`.
			 *
			 * @return
			 *     then statement
			 *
			 */
			const statement& then_statement() const noexcept
			{
				return *_then_branch;
			}

			/**
			 * @brief
			 *     Returns the statement which is executed if the condition
			 *     evaluates to `false`.
			 *
			 * May return `nullptr` in case the `else` branch was omitted.
			 *
			 * @return
			 *     else statement
			 *
			 */
			const statement* else_statement() const noexcept
			{
				return _else_branch.get();
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
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
		 * @brief
		 *     While statement AST node
		 */
		class while_statement final : public statement
		{

		public:

			/**
			 * @brief
			 *     Constructs a while statement node.
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

			/**
			 * @brief
			 *     Returns the loop condition.
			 *
			 * @return
			 *     loop condition
			 *
			 */
			const expression& condition() const noexcept
			{
				return *_condition;
			}

			/**
			 * @brief
			 *     Returns the body of this loop.
			 *
			 * @return
			 *     loop body
			 *
			 */
			const statement& body() const noexcept
			{
				return *_body;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief loop condition */
			std::unique_ptr<expression> _condition;

			/** @brief loop body */
			std::unique_ptr<statement> _body;
		};

		/**
		 * @brief
		 *     Return statement AST node
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

			/**
			 * @brief
			 *     Returns the value to be returned.
			 *
			 * May return `nullptr` in case `void` is returned.
			 *
			 * @return
			 *     return value
			 *
			 */
			const expression* value() const noexcept
			{
				return _value.get();
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief return value or null pointer if void is returned */
			std::unique_ptr<expression> _value;
		};

		/**
		 * @brief
		 *     Empty statement AST node
		 */
		class empty_statement final : public statement
		{

		public:

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}
		};

		// endregion


		// region structural elements

		/**
		 * @brief
		 *     Main method AST node
		 */
		class main_method final : public node
		{

		public:

			/**
			 * @brief
			 *     Constructs a main method node.
			 *
			 * @param main
			 *     method name (usually `main`)
			 *
			 * @param args
			 *     parameter name (usually `args`)
			 *
			 * @param body
			 *     method body
			 *
			 */
			main_method(symbol main, symbol args, std::unique_ptr<block> body)
				: _name{main}, _argname{args}, _body{std::move(body)}
			{
				assert(!_name.empty());
				assert(_body);
			}

			/**
			 * @brief
			 *     Returns the name of this method.
			 *
			 * @return
			 *     method name
			 *
			 */
			symbol name() const noexcept
			{
				return _name;
			}

			/**
			 * @brief
			 *     Returns the name of the declared parameter.
			 *
			 * @return
			 *     name of declared parameter
			 *
			 */
			symbol argname() const noexcept
			{
				return _argname;
			}

			/**
			 * @brief
			 *     Returns the body of this method.
			 *
			 * @return
			 *     method body
			 *
			 */
			const block& body() const noexcept
			{
				return *_body;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief method name */
			symbol _name;

			/** @brief Declared paramter name */
			symbol _argname;

			/** @brief method body */
			std::unique_ptr<block> _body;
		};

		/**
		 * @brief
		 *     Method AST node
		 */
		class method final : public node
		{

		public:

			/**
			 * @brief
			 *     Constructs a method node.
			 *
			 * @param name
			 *     method name
			 *
			 * @param return_type
			 *     method return type
			 *
			 * @param parameters
			 *     method parameters
			 *
			 * @param body
			 *     method body
			 *
			 */
			method(symbol name, std::unique_ptr<type> return_type,
				   std::vector<std::unique_ptr<var_decl>> parameters,
				   std::unique_ptr<block> body)
					: _name{name}, _return_type{std::move(return_type)},
					  _parameters{std::move(parameters)}, _body{std::move(body)}
			{
				assert(!_name.empty());
				assert(_return_type);
				assert(std::all_of(_parameters.begin(), _parameters.end(), [](auto&& el) { return !!el; }));
				assert(_body);
			}

			/**
			 * @brief
			 *     Returns the name of this method.
			 *
			 * @return
			 *     method name
			 *
			 */
			symbol name() const noexcept
			{
				return _name;
			}

			/**
			 * @brief
			 *     Returns the return type of this method.
			 *
			 * @return
			 *     return type
			 *
			 */
			const type& return_type() const noexcept
			{
				return *_return_type;
			}

			/**
			 * @brief
			 *     Returns the parameter list of this method.
			 *
			 * @return
			 *     parameter list
			 *
			 */
			const std::vector<std::unique_ptr<var_decl>>& parameters() const noexcept
			{
				return _parameters;
			}

			/**
			 * @brief
			 *     Returns the body of this method.
			 *
			 * @return
			 *     method body
			 *
			 */
			const block& body() const noexcept
			{
				return *_body;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief method name */
			symbol _name;

			/** @brief method return type */
			std::unique_ptr<type> _return_type;

			/** @brief method parameters */
			std::vector<std::unique_ptr<var_decl>> _parameters;

			/** @brief method body */
			std::unique_ptr<block> _body;
		};

		/**
		 * @brief
		 *     Class declaration AST node
		 */
		class class_declaration final : public node
		{

		public:

			/**
			 * @brief
			 *     Constructs a class declaration node.
			 *
			 * @param name
			 *     class name
			 *
			 * @param fields
			 *     fields inside the class
			 *
			 * @param methods
			 *     class methods
			 *
			 * @param main_methods
			 *     main methods inside the class
			 *
			 */
			class_declaration(symbol name,
			                  std::vector<std::unique_ptr<var_decl>> fields,
			                  std::vector<std::unique_ptr<method>> methods,
			                  std::vector<std::unique_ptr<main_method>> main_methods)
					: _name{name}, _fields{std::move(fields)},
					  _methods{std::move(methods)},
					  _main_methods{std::move(main_methods)}
			{
				assert(!_name.empty());
				assert(std::all_of(_fields.begin(), _fields.end(), [](auto&& el){ return !!el; }));
				assert(std::all_of(_methods.begin(), _methods.end(), [](auto&& el){ return !!el; }));
				assert(std::all_of(_main_methods.begin(), _main_methods.end(), [](auto&& el){ return !!el; }));
			}

			/**
			 * @brief
			 *     Returns the name of this class.
			 *
			 * @return
			 *     class name
			 *
			 */
			symbol name() const noexcept
			{
				return _name;
			}

			/**
			 * @brief
			 *     Returns the fields declared in this class.
			 *
			 * @return
			 *     list of fields
			 *
			 */
			const std::vector<std::unique_ptr<var_decl>>& fields() const noexcept
			{
				return _fields;
			}

			/**
			 * @brief
			 *     Returns the methods declared in this class.
			 *
			 * @return
			 *     list of methods
			 *
			 */
			const std::vector<std::unique_ptr<method>>& methods() const noexcept
			{
				return _methods;
			}

			/**
			 * @brief
			 *     Returns the main methods declared in this class.
			 *
			 * @return
			 *     list of main methods
			 *
			 */
			const std::vector<std::unique_ptr<main_method>>& main_methods() const noexcept
			{
				return _main_methods;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief class name */
			symbol _name;

			/** @brief declared fields */
			std::vector<std::unique_ptr<var_decl>> _fields;

			/** @brief declared methods */
			std::vector<std::unique_ptr<method>> _methods;

			/** @brief declared main methods */
			std::vector<std::unique_ptr<main_method>> _main_methods;
		};

		/**
		 * @brief
		 *     Root node of the AST
		 */
		class program final : public node
		{

		public:

			/**
			 * @brief
			 *     Constructs a root node.
			 *
			 * @param classes
			 *     classes in the program
			 *
			 */
			program(std::vector<std::unique_ptr<class_declaration>> classes)
					: _classes{std::move(classes)}
			{
				assert(std::all_of(_classes.begin(), _classes.end(), [](auto&& el){ return !!el; }));
			}

			/**
			 * @brief
			 *     Returns the classes declared in this program.
			 *
			 * @return
			 *     list of classes
			 *
			 */
			const std::vector<std::unique_ptr<class_declaration>>& classes() const noexcept
			{
				return _classes;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief classes declared in this program */
			std::vector<std::unique_ptr<class_declaration>> _classes;
		};

		// endregion
	}
}

#define MINIJAVA_INCLUDED_FROM_PARSER_AST_HPP
#include "parser/ast.tpp"
#undef MINIJAVA_INCLUDED_FROM_PARSER_AST_HPP
