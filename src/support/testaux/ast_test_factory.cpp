#include "testaux/ast_test_factory.hpp"

#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;

using pool_type = minijava::symbol_pool<>;
using factory_type = minijava::ast_factory;


namespace testaux
{

	std::unique_ptr<ast::main_method>
	make_empty_main(const std::string& name,
					minijava::symbol_pool<>& pool,
					minijava::ast_factory& factory)
	{
		return factory.make<ast::main_method>()(
			pool.normalize(name),
			pool.normalize("args"),
			factory.make<ast::block>()(
				make_unique_ptr_vector<ast::block_statement>()
			)
		);
	}

	std::unique_ptr<ast::class_declaration>
	make_empty_class(const std::string& name, pool_type& pool)
	{
		return std::make_unique<ast::class_declaration>(
				pool.normalize(name),
				make_unique_ptr_vector<ast::var_decl>(),
				make_unique_ptr_vector<ast::instance_method>(),
				make_unique_ptr_vector<ast::main_method>()
		);
	}

	std::unique_ptr<ast::class_declaration>
	make_empty_class(const std::string& name,
					 pool_type& pool, factory_type& factory)
	{
		return factory.make<ast::class_declaration>()(
				pool.normalize(name),
				make_unique_ptr_vector<ast::var_decl>(),
				make_unique_ptr_vector<ast::instance_method>(),
				make_unique_ptr_vector<ast::main_method>()
		);
	}

	std::unique_ptr<ast::class_declaration>
	as_class(const std::string& name, std::unique_ptr<ast::main_method> method,
			 pool_type& pool, factory_type& factory)
	{
		return factory.make<ast::class_declaration>()(
			pool.normalize(name),
			make_unique_ptr_vector<ast::var_decl>(),
			make_unique_ptr_vector<ast::instance_method>(),
			make_unique_ptr_vector<ast::main_method>(std::move(method))
		);
	}

	std::unique_ptr<ast::program>
	as_program(std::unique_ptr<ast::main_method> method, pool_type& pool, factory_type& factory)
	{
		return factory.make<ast::program>()(
			make_unique_ptr_vector<ast::class_declaration>(
				as_class("Test", std::move(method), pool, factory)
			)
		);
	}

	std::unique_ptr<ast::program>
	make_hello_world(const std::string& name, pool_type& pool, factory_type& factory)
	{
		return factory.make<ast::program>()(
			make_unique_ptr_vector<ast::class_declaration>(
				factory.make<ast::class_declaration>()(
					pool.normalize(name),
					make_unique_ptr_vector<ast::var_decl>(),
					make_unique_ptr_vector<ast::instance_method>(),
					make_unique_ptr_vector<ast::main_method>(
						make_empty_main("main", pool, factory)
					)
				)
			)
		);
	}

}
