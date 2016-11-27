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
#include <position.hpp>

#include "symbol/symbol.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     `namespace` for AST node types.
	 *
	 */
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
		class method;
		class main_method;
		class instance_method;
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

			/**
			 * @brief
			 *     Visits a method AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit_method(const method& node);

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
			 *     Visits an instance_method AST node.
			 *
			 * @param node
			 *     The node
			 */
			virtual void visit(const instance_method& node);

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
			 *     A helper `class` to mutate fields of `node`s that should
			 *     normally not be mutatet.
			 *
			 * This is a low-level helper used to break encapsulation in a
			 * controlled way.  Stay away from it and use factories and
			 * builders instead.
			 *
			 * In order to mutate a `node`'s fields, create a `node::mutator`
			 * object, set its fields to the desired values and then call the
			 * function call operator on it, passing it the `node` that should
			 * be mutated.
			 *
			 */
			struct mutator
			{
				/** @brief Desired value for the `node`'s `id` attribute. */
				std::size_t id{};

				/** @brief Desired value for the `node`'s `position` attribute. */
				minijava::position position{};

				/**
				 * @brief
				 *     Sets the attributes of the `node` to the attributes set
				 *     on this object.
				 *
				 * @param n
				 *     `node` to have its attributes set
				 *
				 */
				void operator()(node& n) const noexcept
				{
					n._id = id;
					n._position = position;
				}

			};  // struct mutator

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
			 *     `return`s an optional ID for this `node` in the AST.
			 *
			 * AST nodes never make use of this information themselves but
			 * client code might use it to quickly associate arbitrary
			 * information with AST nodes.  The nodes in an AST shall either
			 * have all ID zero or else each have a unique non-zero ID.
			 *
			 * @returns
			 *     the optional ID of this `node`
			 *
			 */
			std::size_t id() const noexcept {
				return _id;
			}

			/**
			 * @brief
			 *     `return`s the position where the node was found.
			 *
			 * If the value is (0,0), the position is unknown.
			 *
			 * @returns
			 *     position
			 *
			 */
			minijava::position position() const noexcept {
				return _position;
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

            /** @brief Optional ID of this node in the AST. */
			std::size_t _id {};

			/** @brief position number where the operation was found. */
			minijava::position _position {};
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
							  std::vector<std::unique_ptr<expression>> arguments);

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
			 * The `negative` flag allows to distinguish `-1` and `-(1)` in the
			 * AST.
			 *
			 * @param literal
			 *     integer literal specifying the value of the constant
			 *
			 * @param negative
			 *     whether the literal is negated
			 *
			 */
			integer_constant(symbol literal, bool negative = false)
				: _literal{literal}, _negative{negative}
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

			/**
			 * @brief
			 *     Returns whether the literal is negated.
			 *
			 * @return
			 *     whether the literal is negated
			 *
			 */
			bool negative() const noexcept
			{
				return _negative;
			}

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief integer literal specifying the value of the constant */
			symbol _literal;

			/** @brief flag whether the literal is negated */
			bool _negative;

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
			block(std::vector<std::unique_ptr<block_statement>> statements);

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
		 *     Base class for both types of method nodes.
		 */
		class method : public node
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
			       std::unique_ptr<block> body);

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
		 *     Main method AST node
		 */
		class main_method final : public method
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
				: method{main, std::make_unique<type>(primitive_type::type_void), {}, std::move(body)},
				  _argname{args}
			{
				assert(!_argname.empty());
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

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief method name */
			symbol _name;

			/** @brief declared parameter name */
			symbol _argname;

			/** @brief method body */
			std::unique_ptr<block> _body;
		};

		/**
		 * @brief
		 *     Instance method AST node
		 */
		class instance_method final : public method
		{

		public:

			using method::method;

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

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
			 *     instance methods inside the class
			 *
			 * @param main_methods
			 *     main methods inside the class
			 *
			 */
			class_declaration(symbol name,
			                  std::vector<std::unique_ptr<var_decl>> fields,
			                  std::vector<std::unique_ptr<instance_method>> methods,
			                  std::vector<std::unique_ptr<main_method>> main_methods);

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
			 * The list is sorted according to the order defined by
			 * `minijava::symbol_comparator`.
			 *
			 * @return
			 *     sorted list of fields
			 *
			 */
			const std::vector<std::unique_ptr<var_decl>>& fields() const noexcept
			{
				return _fields;
			}

			/**
			 * @brief
			 *     Finds all fields with the given name in this class.
			 *
			 * There may be more than one field with the given name or none.
			 *
			 * @param name
			 *     name of the field
			 *
			 * @return
			 *     range of matching fields
			 *
			 */
			std::pair<const std::unique_ptr<var_decl>*, const std::unique_ptr<var_decl>*>
			find_fields(symbol name) const noexcept;

			/**
			 * @brief
			 *     Finds zero or one fields with the given name.
			 *
			 * If there is no match, `return`s `nullptr`.  If there is exactly
			 * one match, `return`s a pointer to it.  Otherwise, if there is
			 * more than one match, `throws` a `std::out_of_range` exception.
			 *
			 * @param name
			 *     name to find
			 *
			 * @return
			 *     pointer to unique match or `nullptr` if none
			 *
			 * @throws std::out_of_range
			 *     if there are multiple matches
			 *
			 */
			const var_decl*
			get_field(symbol name) const;

			/**
			 * @brief
			 *     Returns the instance methods declared in this class.
			 *
			 * The list is sorted according to the order defined by
			 * `minijava::symbol_comparator`.
			 *
			 * @return
			 *     list of instance methods
			 *
			 */
			const std::vector<std::unique_ptr<instance_method>>& instance_methods() const noexcept
			{
				return _methods;
			}

			/**
			 * @brief
			 *     Finds all instance methods with the given name in this class.
			 *
			 * There may be more than one method with the given name or none.
			 *
			 * @param name
			 *     name of the method
			 *
			 * @return
			 *     range of matching instance methods
			 *
			 */
			std::pair<const std::unique_ptr<instance_method>*, const std::unique_ptr<instance_method>*>
			find_instance_methods(symbol name) const noexcept;

			/**
			 * @brief
			 *     Finds zero or one instance methods with the given name.
			 *
			 * If there is no match, `return`s `nullptr`.  If there is exactly
			 * one match, `return`s a pointer to it.  Otherwise, if there is
			 * more than one match, `throws` a `std::out_of_range` exception.
			 *
			 * @param name
			 *     name to find
			 *
			 * @return
			 *     pointer to unique match or `nullptr` if none
			 *
			 * @throws std::out_of_range
			 *     if there are multiple matches
			 *
			 */
			const instance_method*
			get_instance_method(symbol name) const;

			/**
			 * @brief
			 *     Returns the main methods declared in this class.
			 *
			 * The list is sorted according to the order defined by
			 * `minijava::symbol_comparator`.
			 *
			 * @return
			 *     list of main methods
			 *
			 */
			const std::vector<std::unique_ptr<main_method>>& main_methods() const noexcept
			{
				return _main_methods;
			}

			/**
			 * @brief
			 *     Finds all main methods with the given name in this class.
			 *
			 * There may be more than one method with the given name or none.
			 *
			 * @param name
			 *     name of the method
			 *
			 * @return
			 *     range of matching main methods
			 *
			 */
			std::pair<const std::unique_ptr<main_method>*, const std::unique_ptr<main_method>*>
			find_main_methods(symbol name) const noexcept;

			/**
			 * @brief
			 *     Finds zero or one main methods with the given name.
			 *
			 * If there is no match, `return`s `nullptr`.  If there is exactly
			 * one match, `return`s a pointer to it.  Otherwise, if there is
			 * more than one match, `throws` a `std::out_of_range` exception.
			 *
			 * @param name
			 *     name to find
			 *
			 * @return
			 *     pointer to unique match or `nullptr` if none
			 *
			 * @throws std::out_of_range
			 *     if there are multiple matches
			 *
			 */
			const main_method*
			get_main_method(symbol name) const;

			void accept(visitor& v) const override
			{
				v.visit(*this);
			}

		private:

			/** @brief class name */
			symbol _name;

			/** @brief declared fields */
			std::vector<std::unique_ptr<var_decl>> _fields;

			/** @brief declared instance methods */
			std::vector<std::unique_ptr<instance_method>> _methods;

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
			program(std::vector<std::unique_ptr<class_declaration>> classes);

			/**
			 * @brief
			 *     Returns the classes declared in this program.
			 *
			 * The list is sorted according to the order defined by
			 * `minijava::symbol_comparator`.
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

	}  // namespace ast

}  // namespace minijava
