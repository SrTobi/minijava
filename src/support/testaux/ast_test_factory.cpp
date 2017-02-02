#include "testaux/ast_test_factory.hpp"

#include <cstdint>
#include <cstdlib>

#include "testaux/random_tokens.hpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;


namespace testaux
{

	std::unique_ptr<ast::integer_constant>
	ast_test_factory::make_integer(const std::string& lexval, const bool negative)
	{
		return factory.make<ast::integer_constant>()(pool.normalize(lexval), negative);
	}

	std::unique_ptr<ast::integer_constant>
	ast_test_factory::make_integer(std::int32_t intval)
	{
		const auto sig = (intval < 0);
		const auto mag = std::abs(std::int64_t{intval});
		const auto lexval = std::to_string(mag);
		return factory.make<ast::integer_constant>()(pool.normalize(lexval), sig);
	}

	std::unique_ptr<ast::boolean_constant>
	ast_test_factory::make_boolean(const bool value)
	{
		return factory.make<ast::boolean_constant>()(value);
	}

	std::unique_ptr<ast::variable_access>
	ast_test_factory::make_idref(const std::string& name)
	{
		return factory.make<ast::variable_access>()(nox(), pool.normalize(name));
	}

	std::unique_ptr<ast::variable_access>
	ast_test_factory::make_idref_this(const std::string& name)
	{
		return factory.make<ast::variable_access>()(make_this(), pool.normalize(name));
	}

	std::unique_ptr<ast::method_invocation>
	ast_test_factory::make_call(const std::string& name)
	{
		return factory.make<ast::method_invocation>()(
			nox(),
			pool.normalize(name),
			make_unique_ptr_vector<ast::expression>()
		);
	}

	std::unique_ptr<ast::method_invocation>
	ast_test_factory::make_call_this(const std::string& name)
	{
		return factory.make<ast::method_invocation>()(
			make_this(),
			pool.normalize(name),
			make_unique_ptr_vector<ast::expression>()
		);
	}

	std::unique_ptr<ast::this_ref>
	ast_test_factory::make_this()
	{
		return factory.make<ast::this_ref>()();
	}

	std::unique_ptr<ast::null_constant>
	ast_test_factory::make_null()
	{
		return factory.make<ast::null_constant>()();
	}

	std::unique_ptr<ast::object_instantiation>
	ast_test_factory::make_new(const std::string& name)
	{
		return factory.make<ast::object_instantiation>()
			(pool.normalize(name));
	}

	std::unique_ptr<ast::unary_expression>
	ast_test_factory::logical_not(std::unique_ptr<ast::expression> expr)
	{
		return factory.make<ast::unary_expression>()
			(ast::unary_operation_type::logical_not, std::move(expr));
	}

	std::unique_ptr<ast::unary_expression>
	ast_test_factory::minus(std::unique_ptr<ast::expression> expr)
	{
		return factory.make<ast::unary_expression>()
			(ast::unary_operation_type::minus, std::move(expr));
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::logical_or(std::unique_ptr<ast::expression> lhs,
								 std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::logical_or,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::logical_and(std::unique_ptr<ast::expression> lhs,
								 std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::logical_and,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::equal(std::unique_ptr<ast::expression> lhs,
	                        std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::equal,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::not_equal(std::unique_ptr<ast::expression> lhs,
								 std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::not_equal,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::less_than(std::unique_ptr<ast::expression> lhs,
								 std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::less_than,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::less_equal(std::unique_ptr<ast::expression> lhs,
								 std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::less_equal,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::greater_than(std::unique_ptr<ast::expression> lhs,
								 std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::greater_than,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::greater_equal(std::unique_ptr<ast::expression> lhs,
								 std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::greater_equal,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::plus(std::unique_ptr<ast::expression> lhs,
	                        std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::plus,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::minus(std::unique_ptr<ast::expression> lhs,
	                        std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::minus,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::multiply(std::unique_ptr<ast::expression> lhs,
	                        std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::multiply,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::divide(std::unique_ptr<ast::expression> lhs,
	                        std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::divide,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::modulo(std::unique_ptr<ast::expression> lhs,
								 std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::modulo,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::binary_expression>
	ast_test_factory::assign(std::unique_ptr<ast::expression> lhs,
	                         std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::binary_expression>()(
			ast::binary_operation_type::assign,
			std::move(lhs),
			std::move(rhs)
		);
	}

	std::unique_ptr<ast::expression_statement>
	ast_test_factory::make_assignment(std::unique_ptr<ast::expression> lhs,
	                                  std::unique_ptr<ast::expression> rhs)
	{
		return factory.make<ast::expression_statement>()
			(assign(std::move(lhs), std::move(rhs)));
	}

	std::unique_ptr<ast::empty_statement>
	ast_test_factory::make_empty_stmt()
	{
		return factory.make<ast::empty_statement>()();
	}

	std::unique_ptr<ast::return_statement>
	ast_test_factory::make_return()
	{
		return factory.make<ast::return_statement>()(nullptr);
	}

	std::unique_ptr<ast::return_statement>
	ast_test_factory::make_return(std::unique_ptr<ast::expression> expr)
	{
		return factory.make<ast::return_statement>()(std::move(expr));
	}

	std::unique_ptr<ast::block>
	ast_test_factory::make_empty_block()
	{
		return factory.make<ast::block>()(
			make_unique_ptr_vector<ast::block_statement>()
		);
	}

	std::unique_ptr<ast::block>
	ast_test_factory::as_block(std::unique_ptr<ast::block_statement> blkstmt)
	{
		return factory.make<ast::block>()(
			make_unique_ptr_vector<ast::block_statement>(std::move(blkstmt))
		);
	}

	std::unique_ptr<ast::block>
	ast_test_factory::as_block(std::unique_ptr<ast::expression> expr)
	{
		return as_block(factory.make<ast::expression_statement>()(std::move(expr)));
	}

	std::unique_ptr<ast::main_method>
	ast_test_factory::make_empty_main(const std::string& name, const std::string& argname)
	{
		return factory.make<ast::main_method>()(
			pool.normalize(name),
			pool.normalize(argname),
			factory.make<ast::block>()(
				make_unique_ptr_vector<ast::block_statement>()
			)
		);
	}

	std::unique_ptr<ast::instance_method>
	ast_test_factory::make_empty_method(const std::string& name)
	{
		return factory.make<ast::instance_method>()(
			pool.normalize(name),
			factory.make<ast::type>()(ast::primitive_type::type_void),
			make_unique_ptr_vector<ast::var_decl>(),
			factory.make<ast::block>()(
				make_unique_ptr_vector<ast::block_statement>()
			)
		);
	}

	std::unique_ptr<ast::class_declaration>
	ast_test_factory::make_empty_class(const std::string& name)
	{
		return std::make_unique<ast::class_declaration>(
				pool.normalize(name),
				make_unique_ptr_vector<ast::var_decl>(),
				make_unique_ptr_vector<ast::instance_method>(),
				make_unique_ptr_vector<ast::main_method>()
		);
	}

	std::unique_ptr<ast::var_decl>
	ast_test_factory::make_declaration(const std::string& name,
									   const std::string& type,
									   const std::size_t rank)
	{
		return factory.make<ast::var_decl>()(
			factory.make<ast::type>()(pool.normalize(type), rank),
			pool.normalize(name)
		);
	}

	std::unique_ptr<ast::var_decl>
	ast_test_factory::make_declaration(const std::string& name,
									   const ast::primitive_type type,
									   const std::size_t rank)
	{
		return factory.make<ast::var_decl>()(
			factory.make<ast::type>()(type, rank),
			pool.normalize(name)
		);
	}

	std::unique_ptr<ast::class_declaration>
	ast_test_factory::make_empty_class()
	{
		const auto name = "Test" + std::to_string(factory.id() + 1);
		return make_empty_class(name);
	}

	std::unique_ptr<ast::class_declaration>
	ast_test_factory::make_empty_random_class()
	{
		const auto name = get_random_identifier(engine);
		return make_empty_class(name);
	}

	std::unique_ptr<ast::class_declaration>
	ast_test_factory::as_class(const std::string& name, std::unique_ptr<ast::main_method> method)
	{
		return factory.make<ast::class_declaration>()(
			pool.normalize(name),
			make_unique_ptr_vector<ast::var_decl>(),
			make_unique_ptr_vector<ast::instance_method>(),
			make_unique_ptr_vector<ast::main_method>(std::move(method))
		);
	}

	std::unique_ptr<ast::class_declaration>
	ast_test_factory::as_class(const std::string& name, std::unique_ptr<ast::instance_method> method)
	{
		return factory.make<ast::class_declaration>()(
			pool.normalize(name),
			make_unique_ptr_vector<ast::var_decl>(),
			make_unique_ptr_vector<ast::instance_method>(std::move(method)),
			make_unique_ptr_vector<ast::main_method>()
		);
	}

	std::unique_ptr<ast::class_declaration>
	ast_test_factory::as_class(const std::string& name, std::unique_ptr<ast::var_decl> decl)
	{
		return factory.make<ast::class_declaration>()(
			pool.normalize(name),
			make_unique_ptr_vector<ast::var_decl>(std::move(decl)),
			make_unique_ptr_vector<ast::instance_method>(),
			make_unique_ptr_vector<ast::main_method>()
		);
	}

	std::unique_ptr<ast::program>
	ast_test_factory::as_program(std::unique_ptr<ast::class_declaration> clazz)
	{
		return factory.make<ast::program>()(
			make_unique_ptr_vector<ast::class_declaration>(std::move(clazz))
		);
	}

	std::unique_ptr<ast::program>
	ast_test_factory::as_program(std::unique_ptr<ast::main_method> method)
	{
		return factory.make<ast::program>()(
			make_unique_ptr_vector<ast::class_declaration>(
				as_class("Test", std::move(method))
			)
		);
	}

	std::unique_ptr<ast::program>
	ast_test_factory::as_program(std::vector<std::unique_ptr<ast::block_statement>> stmts)
	{
		return as_program(factory.make<ast::block>()(std::move(stmts)));
	}

	std::unique_ptr<ast::program>
	ast_test_factory::as_program(std::unique_ptr<ast::statement> stmt)
	{
		return as_program(as_block(std::move(stmt)));
	}

	std::unique_ptr<ast::program>
	ast_test_factory::as_program(std::unique_ptr<ast::expression> expr)
	{
		return as_program(factory.make<ast::expression_statement>()(std::move(expr)));
	}

	std::unique_ptr<ast::program>
	ast_test_factory::as_program(std::unique_ptr<ast::block> body)
	{
		return as_program(
			factory.make<ast::main_method>()(
				pool.normalize("main"),
				pool.normalize("args"),
				std::move(body)
			)
		);
	}

	std::unique_ptr<ast::program>
	ast_test_factory::make_hello_world(const std::string& name)
	{
		return factory.make<ast::program>()(
			make_unique_ptr_vector<ast::class_declaration>(
				factory.make<ast::class_declaration>()(
					pool.normalize(name),
					make_unique_ptr_vector<ast::var_decl>(),
					make_unique_ptr_vector<ast::instance_method>(),
					make_unique_ptr_vector<ast::main_method>(
						make_empty_main("main")
					)
				)
			)
		);
	}

}
