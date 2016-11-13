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
		class node;
		class type;
		class var_decl;
		class expression;
		class assignment_expression;
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
		 * Base class for AST visitor
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
			inline virtual void visit_node(node& node);

			/**
			 * @brief
			 *     Visits an expression AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit_expression(expression& node);

			/**
			 * @brief
			 *     Visits a constant AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit_constant(constant& node);

			/**
			 * @brief
			 *     Visits a block_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit_block_statement(block_statement& node);

			/**
			 * @brief
			 *     Visits a statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit_statement(statement& node);

		public:
			/**
			 * @brief
			 *     Visits a type AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(type& node);

			/**
			 * @brief
			 *     Visits a var_decl AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(var_decl& node);

			/**
			 * @brief
			 *     Visits an assignment_expression AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(assignment_expression& node);

			/**
			 * @brief
			 *     Visits a binary_expression AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(binary_expression& node);

			/**
			 * @brief
			 *     Visits a unary_expression AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(unary_expression& node);

			/**
			 * @brief
			 *     Visits an object_instantiation AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(object_instantiation& node);

			/**
			 * @brief
			 *     Visits an array_instantiation AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(array_instantiation& node);

			/**
			 * @brief
			 *     Visits an array_access AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(array_access& node);

			/**
			 * @brief
			 *     Visits a variable_access AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(variable_access& node);

			/**
			 * @brief
			 *     Visits a method_invocation AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(method_invocation& node);

			/**
			 * @brief
			 *     Visits a this_ref AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(this_ref& node);

			/**
			 * @brief
			 *     Visits a boolean_constant AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(boolean_constant& node);

			/**
			 * @brief
			 *     Visits an integer_constant AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(integer_constant& node);

			/**
			 * @brief
			 *     Visits a null_constant AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(null_constant& node);

			/**
			 * @brief
			 *     Visits a local_variable_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(local_variable_statement& node);

			/**
			 * @brief
			 *     Visits an expression_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(expression_statement& node);

			/**
			 * @brief
			 *     Visits a block AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(block& node);

			/**
			 * @brief
			 *     Visits an if_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(if_statement& node);

			/**
			 * @brief
			 *     Visits a while_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(while_statement& node);

			/**
			 * @brief
			 *     Visits a return_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(return_statement& node);

			/**
			 * @brief
			 *     Visits an empty_statement AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(empty_statement& node);

			/**
			 * @brief
			 *     Visits a main_method AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(main_method& node);

			/**
			 * @brief
			 *     Visits a method AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(method& node);

			/**
			 * @brief
			 *     Visits a class_declaration AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(class_declaration& node);

			/**
			 * @brief
			 *     Visits a program AST node.
			 *
			 * @param node
			 *     The node
			 */
			inline virtual void visit(program& node);

		};

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

			/**
			 * @brief
			 *     Accepts a visitor
			 *
			 * @param v
			 *     The visitor
			 */
			virtual void accept(visitor& v) = 0;

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
			 *
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
			 *
			 */
			type(symbol type, std::size_t rank)
					: node{}, _type{type}, _rank{rank}
			{
				assert(!_type.empty());
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
			minijava::ast::type_name& type_name() noexcept
			{
				return _type;
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
			const minijava::ast::type_name& type_name() const noexcept
			{
				return _type;
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

			void accept(visitor& v) override
			{
				v.visit(*this);
			}

		private:

			/** @brief user-defined or built-in type represented by this node */
			minijava::ast::type_name _type;

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
			minijava::ast::type& type()
			{
				return *_type;
			}

			/**
			 * @brief
			 *     Returns the type of this variable.
			 *
			 * @return
			 *     type of this variable
			 *
			 */
			const minijava::ast::type& type() const
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
			symbol name() const
			{
				return _name;
			}

			void accept(visitor& v) override
			{
				v.visit(*this);
			}

		private:

			/** @brief type of this variable */
			std::unique_ptr<minijava::ast::type> _type;

			/** @brief symbol representing the name of this variable */
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
			 *
			 */
			assignment_expression(std::unique_ptr<expression> lhs,
			                      std::unique_ptr<expression> rhs)
					: _lhs{std::move(lhs)}, _rhs{std::move(rhs)}
			{
				assert(_lhs);
				assert(_rhs);
			}

			/**
			 * @brief
			 *     Returns the expression on the left side of this assignment
			 *
			 * @return
			 *     left side of this assignment
			 *
			 */
			expression& lhs()
			{
				return *_lhs;
			}

			/**
			 * @brief
			 *     Returns the expression on the left side of this assignment
			 *
			 * @return
			 *     left side of this assignment
			 *
			 */
			const expression& lhs() const
			{
				return *_lhs;
			}

			/**
			 * @brief
			 *     Returns the expression on the right side of this assignment
			 *
			 * @return
			 *     right side of this assignment
			 *
			 */
			expression& rhs()
			{
				return *_rhs;
			}

			/**
			 * @brief
			 *     Returns the expression on the right side of this assignment
			 *
			 * @return
			 *     right side of this assignment
			 *
			 */
			const expression& rhs() const
			{
				return *_rhs;
			}

			void accept(visitor& v) override
			{
				v.visit(*this);
			}

		private:

			/** @brief expression on the left side of this assignment */
			std::unique_ptr<expression> _lhs;

			/** @brief expression on the right side of this assignment */
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

			/**
			 * @brief
			 *     Returns the type of this operation
			 * 
			 * @return
			 *     type of this operation
			 *
			 */
			binary_operation_type type() const
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
			expression& lhs()
			{
				return *_lhs;
			}

			/**
			 * @brief
			 *     Returns the expression on the left side of this operation
			 *
			 * @return
			 *     left side of this operation
			 *
			 */
			const expression& lhs() const
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
			expression& rhs()
			{
				return *_rhs;
			}

			/**
			 * @brief
			 *     Returns the expression on the right side of this operation
			 *
			 * @return
			 *     right side of this operation
			 *
			 */
			const expression& rhs() const
			{
				return *_rhs;
			}

			void accept(visitor& v) override
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
			unary_operation_type type() const
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
			expression& target()
			{
				return *_target;
			}

			/**
			 * @brief
			 *     Returns the expression to which this operation applies
			 *
			 * @return
			 *     target of this operation
			 *
			 */
			const expression& target() const
			{
				return *_target;
			}

			void accept(visitor& v) override
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

			/**
			 * @brief
			 *     Returns the name of the class which is bein instantiated.
			 *
			 * @return
			 *     class name
			 *
			 */
			symbol class_name() const
			{
				return _class_name;
			}

			void accept(visitor& v) override
			{
				v.visit(*this);
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

			/**
			 * @brief
			 *     Returns the type being instantiated.
			 *
			 * @return
			 *     type
			 *
			 */
			minijava::ast::type& type()
			{
				return *_type;
			}

			/**
			 * @brief
			 *     Returns the type being instantiated.
			 *
			 * @return
			 *     type
			 *
			 */
			const minijava::ast::type& type() const
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
			expression& extent()
			{
				return *_extent;
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
			const expression& extent() const
			{
				return *_extent;
			}

			void accept(visitor& v) override
			{
				v.visit(*this);
			}

		private:

			/** @brief type to instantiate */
			std::unique_ptr<minijava::ast::type> _type;

			/** @brief expression evaluating to the extent of the first dimension */
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

			/**
			 * @brief
			 *     Returns the target of this array access.
			 *
			 * @return
			 *     target expression
			 *
			 */
			expression& target()
			{
				return *_target;
			}

			/**
			 * @brief
			 *     Returns the target of this array access.
			 *
			 * @return
			 *     target expression
			 *
			 */
			const expression& target() const
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
			expression& index()
			{
				return *_index;
			}

			/**
			 * @brief
			 *     Returns the index expression.
			 *
			 * @return
			 *     index expression
			 *
			 */
			const expression& index() const
			{
				return *_index;
			}

			void accept(visitor& v) override
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
			expression* target()
			{
				return _target.get();
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
			const expression* target() const
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
			symbol name() const
			{
				return _name;
			}

			void accept(visitor& v) override
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
			 * @param arguments
			 *     arguments passed to the method
			 *
			 */
			method_invocation(std::unique_ptr<expression> target, symbol name,
							  std::vector<std::unique_ptr<expression>> arguments)
					: _target{std::move(target)}, _name{std::move(name)}
					, _arguments{std::move(arguments)}
			{
				assert(!_name.empty());
				assert(std::all_of(_arguments.begin(), _arguments.end(), [](auto&& el) { return !!el; }));
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
			expression* target()
			{
				return _target.get();
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
			const expression* target() const
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
			symbol name() const
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
			std::vector<std::unique_ptr<expression>>& arguments()
			{
				return _arguments;
			}

			/**
			 * @brief
			 *     Returns the arguments passed to the method.
			 *
			 * @return
			 *     arguments
			 *
			 */
			const std::vector<std::unique_ptr<expression>>& arguments() const
			{
				return _arguments;
			}

			void accept(visitor& v) override
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
		 * AST node for a reference to `this`
		 */
		class this_ref final : public expression
		{

		public:

			void accept(visitor& v) override
			{
				v.visit(*this);
			}
		};

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

			void accept(visitor& v) override
			{
				v.visit(*this);
			}

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

			/**
			 * @brief
			 *     Returns the integer literal specifying the value of this
			 *     constant.
			 *
			 * @return
			 *     integer literal
			 *
			 */
			symbol literal() const
			{
				return _literal;
			}

			void accept(visitor& v) override
			{
				v.visit(*this);
			}

		private:

			/** @brief integer literal specifying the value of the constant */
			symbol _literal;
		};

		/**
		 * Null constant AST node
		 */
		class null_constant final : public constant
		{

		public:

			void accept(visitor& v) override
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

			/**
			 * @brief
			 *     Returns the declaration of this local variable.
			 *
			 * @return
			 *     variable declaration
			 */
			var_decl& declaration()
			{
				return *_declaration;
			}

			/**
			 * @brief
			 *     Returns the declaration of this local variable.
			 *
			 * @return
			 *     variable declaration
			 */
			const var_decl& declaration() const
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
			expression* initial_value()
			{
				return _initial_value.get();
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
			const expression* initial_value() const
			{
				return _initial_value.get();
			}

			void accept(visitor& v) override
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

			/**
			 * @brief
			 *     Returns the expression contained in this statement.
			 *
			 * @return
			 *     expression
			 *
			 */
			minijava::ast::expression& expression()
			{
				return *_expression;
			}

			/**
			 * @brief
			 *     Returns the expression contained in this statement.
			 *
			 * @return
			 *     expression
			 *
			 */
			const minijava::ast::expression& expression() const
			{
				return *_expression;
			}

			void accept(visitor& v) override
			{
				v.visit(*this);
			}

		private:

			/** @brief expression to interpret as a statement */
			std::unique_ptr<minijava::ast::expression> _expression;
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

			/**
			 * @brief
			 *     Returns the body of this block.
			 *
			 * @return
			 *     block body
			 *
			 */
			std::vector<std::unique_ptr<block_statement>>& body()
			{
				return _body;
			}

			/**
			 * @brief
			 *     Returns the body of this block.
			 *
			 * @return
			 *     block body
			 *
			 */
			const std::vector<std::unique_ptr<block_statement>>& body() const
			{
				return _body;
			}

			void accept(visitor& v) override
			{
				v.visit(*this);
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
			expression& condition()
			{
				return *_condition;
			}

			/**
			 * @brief
			 *     Returns the branch condition.
			 *
			 * @return
			 *     condition
			 *
			 */
			const expression& condition() const
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
			statement& then_statement()
			{
				return *_then_branch;
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
			const statement& then_statement() const
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
			statement* else_statement()
			{
				return _else_branch.get();
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
			const statement* else_statement() const
			{
				return _else_branch.get();
			}

			void accept(visitor& v) override
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

			/**
			 * @brief
			 *     Returns the loop condition.
			 *
			 * @return
			 *     loop condition
			 *
			 */
			expression& condition()
			{
				return *_condition;
			}

			/**
			 * @brief
			 *     Returns the loop condition.
			 *
			 * @return
			 *     loop condition
			 *
			 */
			const expression& condition() const
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
			statement& body()
			{
				return *_body;
			}

			/**
			 * @brief
			 *     Returns the body of this loop.
			 *
			 * @return
			 *     loop body
			 *
			 */
			const statement& body() const
			{
				return *_body;
			}

			void accept(visitor& v) override
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
			expression* value()
			{
				return _value.get();
			}

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
			const expression* value() const
			{
				return _value.get();
			}

			void accept(visitor& v) override
			{
				v.visit(*this);
			}

		private:

			/** @brief return value or null pointer if void is returned */
			std::unique_ptr<expression> _value;
		};

		/**
		 * Empty statement AST node
		 */
		class empty_statement final : public statement
		{

		public:

			void accept(visitor& v) override
			{
				v.visit(*this);
			}
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

			/**
			 * @brief
			 *     Returns the name of this method.
			 *
			 * @return
			 *     method name
			 *
			 */
			symbol name() const
			{
				return _name;
			}

			/**
			 * @brief
			 *     Returns the body of this method.
			 *
			 * @return
			 *     method body
			 *
			 */
			block& body()
			{
				return *_body;
			}

			/**
			 * @brief
			 *     Returns the body of this method.
			 *
			 * @return
			 *     method body
			 *
			 */
			const block& body() const
			{
				return *_body;
			}

			void accept(visitor& v) override
			{
				v.visit(*this);
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
			symbol name() const
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
			type& return_type()
			{
				return *_return_type;
			}

			/**
			 * @brief
			 *     Returns the return type of this method.
			 *
			 * @return
			 *     return type
			 *
			 */
			const type& return_type() const
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
			std::vector<std::unique_ptr<var_decl>>& parameters()
			{
				return _parameters;
			}

			/**
			 * @brief
			 *     Returns the parameter list of this method.
			 *
			 * @return
			 *     parameter list
			 *
			 */
			const std::vector<std::unique_ptr<var_decl>>& parameters() const
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
			block& body()
			{
				return *_body;
			}

			/**
			 * @brief
			 *     Returns the body of this method.
			 *
			 * @return
			 *     method body
			 *
			 */
			const block& body() const
			{
				return *_body;
			}

			void accept(visitor& v) override
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
		 * Class declaration AST node
		 */
		class class_declaration final : public node
		{

		public:

			/**
			 * Constructs a class declaration node.
			 *
			 * @param name
			 *     class name
			 */
			class_declaration(symbol name) : _name{name}
			{
				assert(!_name.empty());
			}

			/**
			 * @brief
			 *     Returns the name of this class.
			 *
			 * @return
			 *     class name
			 *
			 */
			symbol name() const
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
			std::vector<std::unique_ptr<var_decl>>& fields()
			{
				return _fields;
			}

			/**
			 * @brief
			 *     Returns the fields declared in this class.
			 *
			 * @return
			 *     list of fields
			 *
			 */
			const std::vector<std::unique_ptr<var_decl>>& fields() const
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
			std::vector<std::unique_ptr<method>>& methods()
			{
				return _methods;
			}

			/**
			 * @brief
			 *     Returns the methods declared in this class.
			 *
			 * @return
			 *     list of methods
			 *
			 */
			const std::vector<std::unique_ptr<method>>& methods() const
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
			std::vector<std::unique_ptr<main_method>>& main_methods()
			{
				return _main_methods;
			}

			/**
			 * @brief
			 *     Returns the main methods declared in this class.
			 *
			 * @return
			 *     list of main methods
			 *
			 */
			const std::vector<std::unique_ptr<main_method>>& main_methods() const
			{
				return _main_methods;
			}

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

			void accept(visitor& v) override
			{
				v.visit(*this);
			}

		private:

			/** @brief class name */
			symbol _name;

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

			/**
			 * @brief
			 *     Returns the classes declared in this program.
			 *
			 * @return
			 *     list of classes
			 *
			 */
			std::vector<std::unique_ptr<class_declaration>>& classes()
			{
				return _classes;
			}

			/**
			 * @brief
			 *     Returns the classes declared in this program.
			 *
			 * @return
			 *     list of classes
			 *
			 */
			const std::vector<std::unique_ptr<class_declaration>>& classes() const
			{
				return _classes;
			}

			void accept(visitor& v) override
			{
				v.visit(*this);
			}

		private:

			/** @brief classes declared in this program */
			std::vector<std::unique_ptr<class_declaration>> _classes {};
		};

		// endregion
	}
}

#define MINIJAVA_INCLUDED_FROM_PARSER_AST_HPP
#include "parser/ast.tpp"
#undef MINIJAVA_INCLUDED_FROM_PARSER_AST_HPP
