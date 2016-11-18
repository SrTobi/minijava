#include "semantic/type_system.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#define BOOST_TEST_MODULE  semantic_type_system
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "symbol/symbol_pool.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token_iterator.hpp"
#include "parser/ast.hpp"
#include "parser/parser.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/symbol_def.hpp"

using namespace minijava::semantic;

type_system get_typesystem(const std::string& sample, def_annotations& def_a, minijava::symbol_pool<>& pool)
{
	auto lex = make_lexer(std::begin(sample), std::end(sample), pool, pool);
	const auto tokfirst = minijava::token_begin(lex);
	const auto toklast = minijava::token_end(lex);

	auto ast = minijava::parse_program(tokfirst, toklast);

	return extract_typesystem(*ast, def_a, pool);
}


BOOST_AUTO_TEST_CASE(type_system_extracts_classes)
{
	auto code = R"(
		class A {}
		class B {}
	)";

	auto pool = minijava::symbol_pool<>{};
	def_annotations def_a{};
	auto typesystem = get_typesystem(code, def_a, pool);
	BOOST_REQUIRE(typesystem.resolve_class(pool.normalize("A")));
	BOOST_REQUIRE(typesystem.resolve_class(pool.normalize("B")));
}

BOOST_AUTO_TEST_CASE(type_system_extracts_member)
{
	auto code = R"(
		class A {
			public int foo;
			public void bar(int x, A y){}
		}
		class B {
			public int foo;
			public B bar(boolean x, A y){}
		}
	)";

	auto pool = minijava::symbol_pool<>{};
	def_annotations def_a{};
	auto typesystem = get_typesystem(code, def_a, pool);
	auto* clazzA = typesystem.resolve_class(pool.normalize("A"));
	auto bar = pool.normalize("bar");
	BOOST_REQUIRE(clazzA);
	BOOST_REQUIRE(clazzA->method(bar));
	BOOST_REQUIRE(clazzA->method(bar)->type() == type_system::t_void());

	auto* clazzB = typesystem.resolve_class(pool.normalize("B"));
	BOOST_REQUIRE(clazzB);
	BOOST_REQUIRE(clazzB->method(bar));
	BOOST_REQUIRE(clazzB->method(bar)->type() == clazzB->type());
}


BOOST_AUTO_TEST_CASE(type_system_rejects_multiple_same_named_classes)
{
	auto code = R"(
		class A {}
		class A {}
	)";

	auto pool = minijava::symbol_pool<>{};
	def_annotations def_a{};

	BOOST_REQUIRE_THROW(get_typesystem(code, def_a, pool), minijava::semantic_error);
}

BOOST_AUTO_TEST_CASE(type_system_rejects_multiple_same_named_fields)
{
	auto code = R"(
		class A {
			public int foo;
			public boolean foo;
		}
	)";

	auto pool = minijava::symbol_pool<>{};
	def_annotations def_a{};

	BOOST_REQUIRE_THROW(get_typesystem(code, def_a, pool), minijava::semantic_error);
}

const std::string failure_codes[] = {
	R"(
		class A {}
		class A {}
	)",
	R"(
		class A {
			public int foo;
			public boolean foo;
		}
	)",
	R"(
		class A {
			public int foo(){}
			public boolean foo(){}
		}
	)",
	R"(
		class A {
			public void[] foo(){}
		}
	)",
	R"(
		class A {
			public void foo;
		}
	)",
	R"(
		class A {
			public void[] foo(){}
		}
	)",
	R"(
		class A {
			public void[] foo;
		}
	)",
	R"(
		class A {
			public void foo(void x){}
		}
	)",
	R"(
		class A {
			public void foo(void[] x){}
		}
	)",
	R"(
		class A {
			public int foo(B x) {}
		}
	)",
	R"(
		class A {
			public B foo;
		}
	)",
};

BOOST_DATA_TEST_CASE(type_system_rejects_wrong_typed_signatures, failure_codes)
{
	auto pool = minijava::symbol_pool<>{};
	def_annotations def_a{};

	BOOST_REQUIRE_THROW(get_typesystem(sample, def_a, pool), minijava::semantic_error);
}
