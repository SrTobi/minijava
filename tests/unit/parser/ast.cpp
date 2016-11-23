#include "parser/ast.hpp"

#include <string>
#include <type_traits>
#include <utility>

#define BOOST_TEST_MODULE  parser_ast
#include <boost/test/unit_test.hpp>

#include "meta/meta.hpp"
#include "symbol/symbol.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/unique_ptr_vector.hpp"

namespace ast = minijava::ast;


namespace /* anonymous */
{

	using all_ast_node_types = minijava::meta::types_t
	<
		ast::node,
		ast::type,
		ast::var_decl,
		ast::expression,
		ast::binary_expression,
		ast::unary_expression,
		ast::object_instantiation,
		ast::array_instantiation,
		ast::array_access,
		ast::variable_access,
		ast::method_invocation,
		ast::this_ref,
		ast::constant,
		ast::boolean_constant,
		ast::integer_constant,
		ast::null_constant,
		ast::block_statement,
		ast::local_variable_statement,
		ast::statement,
		ast::expression_statement,
		ast::block,
		ast::if_statement,
		ast::while_statement,
		ast::return_statement,
		ast::empty_statement,
		ast::method,
		ast::main_method,
		ast::instance_method,
		ast::class_declaration,
		ast::program
	>;

	template<typename T>
	struct is_abstract_or_final : std::integral_constant<bool, std::is_abstract<T>{} || std::is_final<T>{}>
	{};

	struct vertex : ast::node
	{

		void accept(ast::visitor&) const override
		{
		}

	};

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(type_checks)
{
	constexpr auto ants = all_ast_node_types{};
	static_assert(minijava::meta::all<std::is_class>(ants), "");
	static_assert(minijava::meta::all<std::has_virtual_destructor>(ants), "");
	static_assert(minijava::meta::all<is_abstract_or_final>(ants), "");
	static_assert(minijava::meta::none<std::is_copy_constructible>(ants), "");
	static_assert(minijava::meta::none<std::is_copy_assignable>(ants), "");
	static_assert(minijava::meta::none<std::is_move_constructible>(ants), "");
	static_assert(minijava::meta::none<std::is_move_assignable>(ants), "");
}


BOOST_AUTO_TEST_CASE(node_has_zero_id_after_construction)
{
	vertex v{};
	BOOST_REQUIRE_EQUAL(0, v.id());
}


BOOST_AUTO_TEST_CASE(node_has_unknown_sloc_after_construction)
{
	vertex v{};
	BOOST_REQUIRE_EQUAL(0, v.line());
	BOOST_REQUIRE_EQUAL(0, v.column());
}


BOOST_AUTO_TEST_CASE(node_can_be_mutated_with_mutator)
{
	vertex v{};
	ast::node::mutator m{1, 2, 3};
	m(v);
	BOOST_REQUIRE_EQUAL(1, v.id());
	BOOST_REQUIRE_EQUAL(2, v.line());
	BOOST_REQUIRE_EQUAL(3, v.column());
}

namespace /* anonymous */
{
	std::unique_ptr<ast::var_decl>
	make_void_field(const char* name, minijava::symbol_pool<>& pool) {
		return std::make_unique<ast::var_decl>(
				std::make_unique<ast::type>(ast::primitive_type::type_void),
				pool.normalize(name)
		);
	}

	std::unique_ptr<ast::instance_method>
	make_empty_method(const char* name, minijava::symbol_pool<>& pool) {
		return std::make_unique<ast::instance_method>(
				pool.normalize(name),
				std::make_unique<ast::type>(ast::primitive_type::type_void),
				std::vector<std::unique_ptr<ast::var_decl>>{},
				std::make_unique<ast::block>(
						std::vector<std::unique_ptr<ast::block_statement>>{}
				)
		);
	}

	std::unique_ptr<ast::main_method>
	make_empty_main_method(const char* name, minijava::symbol_pool<>& pool) {
		return std::make_unique<ast::main_method>(
				pool.normalize(name),
				pool.normalize("args"),
				std::make_unique<ast::block>(
						std::vector<std::unique_ptr<ast::block_statement>>{}
				)
		);
	}

	std::unique_ptr<ast::class_declaration>
	make_empty_class(const char* name, minijava::symbol_pool<>& pool) {
		return std::make_unique<ast::class_declaration>(
				pool.normalize(name),
				testaux::make_unique_ptr_vector<ast::var_decl>(),
				testaux::make_unique_ptr_vector<ast::instance_method>(),
				testaux::make_unique_ptr_vector<ast::main_method>()
		);
	}

	template<typename AstT>
	struct node_comparator {
		const minijava::symbol_comparator sym_cmp{};

		bool operator()(const std::unique_ptr<AstT>& n1,
						const std::unique_ptr<AstT>& n2) {
			return sym_cmp(n1->name(), n2->name());
		}
	};
}

BOOST_AUTO_TEST_CASE(class_methods_and_fields_sorted)
{
	auto pool = minijava::symbol_pool<>{};
	auto clazz = std::make_unique<ast::class_declaration>(
			pool.normalize("test"),
			testaux::make_unique_ptr_vector<ast::var_decl>(
					make_void_field("test1", pool),
					make_void_field("test2", pool),
					make_void_field("test1", pool)
			),
			testaux::make_unique_ptr_vector<ast::instance_method>(
					make_empty_method("test1", pool),
					make_empty_method("test2", pool),
					make_empty_method("test1", pool)
			),
			testaux::make_unique_ptr_vector<ast::main_method>(
					make_empty_main_method("test1", pool),
					make_empty_main_method("test2", pool),
					make_empty_main_method("test1", pool),
					make_empty_main_method("test1", pool)
			)
	);
	BOOST_CHECK(std::is_sorted(
			std::begin(clazz->fields()), std::end(clazz->fields()),
			node_comparator<ast::var_decl>{}
	));
	BOOST_CHECK(std::is_sorted(
			std::begin(clazz->instance_methods()),
			std::end(clazz->instance_methods()),
			node_comparator<ast::instance_method>{}
	));
	BOOST_CHECK(std::is_sorted(
			std::begin(clazz->main_methods()), std::end(clazz->main_methods()),
			node_comparator<ast::main_method>{}
	));
}

BOOST_AUTO_TEST_CASE(class_find_methods_and_find_fields_return_correct_range)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	auto clazz = std::make_unique<ast::class_declaration>(
			pool.normalize("test"),
			testaux::make_unique_ptr_vector<ast::var_decl>(
					make_void_field("test1", pool),
					make_void_field("test2", pool),
					make_void_field("test1", pool)
			),
			testaux::make_unique_ptr_vector<ast::instance_method>(),
			testaux::make_unique_ptr_vector<ast::main_method>()
	);
	auto field_range_1 = clazz->find_fields(pool.normalize("test1"));
	BOOST_REQUIRE_EQUAL(field_range_1.first + 2, field_range_1.second);
	BOOST_CHECK_EQUAL((*field_range_1.first)->name(), "test1"s);
	BOOST_CHECK_EQUAL((*(field_range_1.first + 1))->name(), "test1"s);
	auto field_range_2 = clazz->find_fields(pool.normalize("test2"));
	BOOST_REQUIRE_EQUAL(field_range_2.first + 1, field_range_2.second);
	BOOST_CHECK_EQUAL((*field_range_2.first)->name(), "test2"s);
	auto field_range_3 = clazz->find_fields(pool.normalize("test3"));
	BOOST_CHECK_EQUAL(field_range_3.first, field_range_3.second);
	auto method_range = clazz->find_instance_methods(pool.normalize("foo"));
	BOOST_CHECK_EQUAL(method_range.first, method_range.second);
	auto main_method_range = clazz->find_main_methods(pool.normalize("foo"));
	BOOST_CHECK_EQUAL(main_method_range.first, main_method_range.second);
	auto clazz2 = std::make_unique<ast::class_declaration>(
			pool.normalize("test"),
			testaux::make_unique_ptr_vector<ast::var_decl>(),
			testaux::make_unique_ptr_vector<ast::instance_method>(
					make_empty_method("test1", pool),
					make_empty_method("test2", pool),
					make_empty_method("test1", pool)
			),
			testaux::make_unique_ptr_vector<ast::main_method>(
					make_empty_main_method("test1", pool),
					make_empty_main_method("test2", pool),
					make_empty_main_method("test1", pool),
					make_empty_main_method("test1", pool)
			)
	);
	auto method_range_1 = clazz2->find_instance_methods(pool.normalize("test1"));
	BOOST_REQUIRE_EQUAL(method_range_1.first + 2, method_range_1.second);
	BOOST_CHECK_EQUAL((*method_range_1.first)->name(), "test1"s);
	BOOST_CHECK_EQUAL((*(method_range_1.first + 1))->name(), "test1"s);
	auto method_range_2 = clazz2->find_instance_methods(pool.normalize("test2"));
	BOOST_REQUIRE_EQUAL(method_range_2.first + 1, method_range_2.second);
	BOOST_CHECK_EQUAL((*method_range_2.first)->name(), "test2"s);
	auto method_range_3 = clazz2->find_instance_methods(pool.normalize("test3"));
	BOOST_CHECK_EQUAL(method_range_3.first, method_range_3.second);
	auto field_range = clazz2->find_fields(pool.normalize("foo"));
	BOOST_CHECK_EQUAL(field_range.first, field_range.second);
	auto main_range_1 = clazz2->find_main_methods(pool.normalize("test1"));
	BOOST_REQUIRE_EQUAL(main_range_1.first + 3, main_range_1.second);
	BOOST_CHECK_EQUAL((*main_range_1.first)->name(), "test1"s);
	BOOST_CHECK_EQUAL((*(main_range_1.first + 1))->name(), "test1"s);
	auto main_range_2 = clazz2->find_main_methods(pool.normalize("test2"));
	BOOST_REQUIRE_EQUAL(main_range_2.first + 1, main_range_2.second);
	BOOST_CHECK_EQUAL((*main_range_2.first)->name(), "test2"s);
	auto main_range_3 = clazz2->find_main_methods(pool.normalize("test3"));
	BOOST_CHECK_EQUAL(main_range_3.first, main_range_3.second);
}

BOOST_AUTO_TEST_CASE(program_classes_sorted)
{
	auto pool = minijava::symbol_pool<>{};
	auto program = std::make_unique<ast::program>(
			testaux::make_unique_ptr_vector<ast::class_declaration>(
					make_empty_class("test1", pool),
					make_empty_class("test1", pool),
					make_empty_class("test2", pool),
					make_empty_class("test2", pool),
					make_empty_class("test3", pool)
			)
	);
	BOOST_CHECK(std::is_sorted(
			std::begin(program->classes()), std::end(program->classes()),
			node_comparator<ast::class_declaration>{}
	));
}
