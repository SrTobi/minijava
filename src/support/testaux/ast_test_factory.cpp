#include "testaux/ast_test_factory.hpp"

#include "testaux/unique_ptr_vector.hpp"

namespace testaux
{
	std::unique_ptr<minijava::ast::class_declaration>
	make_empty_class(const char* name, minijava::symbol_pool<>& pool)
	{
		return std::make_unique<minijava::ast::class_declaration>(
				pool.normalize(name),
				make_unique_ptr_vector<minijava::ast::var_decl>(),
				make_unique_ptr_vector<minijava::ast::instance_method>(),
				make_unique_ptr_vector<minijava::ast::main_method>()
		);
	}

	std::unique_ptr<minijava::ast::class_declaration>
	make_empty_class(const char* name, minijava::symbol_pool<>& pool,
					 minijava::ast_factory& factory)
	{
		return factory.make<minijava::ast::class_declaration>()(
				pool.normalize(name),
				make_unique_ptr_vector<minijava::ast::var_decl>(),
				make_unique_ptr_vector<minijava::ast::instance_method>(),
				make_unique_ptr_vector<minijava::ast::main_method>()
		);
	}
}
