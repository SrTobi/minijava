#include "testaux/ast_test_factory.hpp"

#include "testaux/random_tokens.hpp"
#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;


namespace testaux
{

	std::unique_ptr<minijava::ast::block>
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

	std::unique_ptr<ast::main_method>
	ast_test_factory::make_empty_main(const std::string& name)
	{
		return factory.make<ast::main_method>()(
			pool.normalize(name),
			pool.normalize("args"),
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
	ast_test_factory::as_program(std::unique_ptr<ast::main_method> method)
	{
		return factory.make<ast::program>()(
			make_unique_ptr_vector<ast::class_declaration>(
				as_class("Test", std::move(method))
			)
		);
	}

	std::unique_ptr<ast::program>
	ast_test_factory::as_program(std::unique_ptr<ast::statement> stmt)
	{
		return as_program(as_block(std::move(stmt)));
	}

	std::unique_ptr<minijava::ast::program>
	ast_test_factory::as_program(std::unique_ptr<minijava::ast::expression> expr)
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
