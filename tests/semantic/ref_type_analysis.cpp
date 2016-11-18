#include "semantic/ref_type_analysis.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#define BOOST_TEST_MODULE  semantic_symbol_type_analysis
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "symbol/symbol_pool.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token_iterator.hpp"
#include "parser/ast.hpp"
#include "parser/parser.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/symbol_def.hpp"
#include "semantic/buildins.hpp"

#include "testaux/testaux.hpp"

namespace buildins = minijava::semantic::buildins;

static const std::string success_data[] = {
	"",
	R"(
		class A {}
		class B {}
	)",
	R"(
		class A {
			public int mem1;
			public boolean mem2;
		}
	)",
	R"(
		class A {
			public int name;
			public int name(){}
		}
	)",
	R"(
		class B {}
		class A {
			public A foo;
			public B bar;
		}
	)",
	R"(
		class A {
			public void test(){}
		}
	)",
	R"(
		class A {
			public int field;

			public void test(boolean a){
				a;
				int b;
				b;
				field;
			}
		}
	)",
	R"(
		class A {
			public int field;

			public void test(A a){
				A b;
				a.field;
				b.field;
			}
		}
	)",
	R"(
		class A {
			public int field;

			public void test(A a){
				A b;
				{
					A c;
					{
						a; b; c;
					}
					a; b; c;
				}
				a; b;
			}
		}
	)",
	R"(
		class A {
			public boolean field;

			public void test(){
				this.field;
				field;
			}
		}
	)",
	R"(
		class A {
			public int foo;
			public void test(){
				boolean foo;
				if(foo)
				{

				}
				while(foo)
				{

				}
			}
		}
	)",
	R"(
		class A {
			public boolean foo;
			public void test()
			{
				{
					int foo;
				}
				if(foo)
				{

				}
			}
		}
	)",
	R"(
		class A {
			public int lol;
			public void test()
			{
				int[] foo;
				foo[0];
				A[][] bar;
				bar[0][0].lol;
			}
		}
	)",
	R"(
		class A {
			public int lol;
			public void test()
			{
				- lol;
				- - lol;
				-(lol);
				boolean a;
				!a;
				!!a;
			}
		}
	)",
	R"(
		class A {
			public int lol;
			public void test()
			{
				lol = 3;
				lol = 4 + 3;
				lol = 9 - 3;
				lol = 3 * 3;
				lol = 3 % 3;
				lol = 3 / 0;
			}
		}
	)",
	R"(
		class A {
			public boolean lol;
			public void test(int x, int y)
			{
				lol = x == 3 && y != 3;
				lol = x < 3 || y > 3;
				lol = x <= 3 == y > 3;
				lol = x <= 3 != true;
			}
		}
	)",
	R"(
		class A {
			public void test()
			{
				return;
			}
		}
	)",
	R"(
		class A {
			public int test(int x)
			{
				return x;
			}
		}
	)",
	R"(
		class A {
			public int foo(boolean a)
			{
			}

			public int test(int x)
			{
				int y = foo(true);
				return test(x);
			}
		}
	)",
	R"(
		class A {
			public int foo(boolean a)
			{
			}

			public int test(int x)
			{
				foo(true);
				return test(x);
			}
		}
	)",
	R"(
		class A {
			public int foo(A a)
			{
			}

			public int test()
			{
				foo(null);
				foo(new A());
			}
		}
	)",
	R"(
		class A {
			public int foo(A a)
			{
				System.out;
				System.out.println(5);
			}
		}
	)",
};

BOOST_DATA_TEST_CASE(symbol_type_analysis_accepts_valid_programs, success_data)
{
	auto pool = minijava::symbol_pool<>{};
	auto lex = make_lexer(std::begin(sample), std::end(sample), pool, pool);
	const auto tokfirst = minijava::token_begin(lex);
	const auto toklast = minijava::token_end(lex);

	auto ast = minijava::parse_program(tokfirst, toklast);

	minijava::semantic::def_annotations def_a{};
	auto typesystem = minijava::semantic::extract_typesystem(*ast, def_a);
	auto system = buildins::register_system(typesystem, pool);
	try{
		minijava::semantic::analyse_program(*ast, {{pool.normalize("System"), system}}, typesystem, def_a);
	}catch(const minijava::semantic_error& e)
	{
		BOOST_FAIL("Exception thrown: " << e.what());
	}
}


static const std::string failure_data[] = {
	R"(
		class A {
			public void foo()
			{
				void xxx;
			}
		}
	)",
	R"(
		class A {
			public void foo()
			{
				void[] xxx;
			}
		}
	)",
	R"(
		class A {
			public void test()
			{
				a;
			}
		}
	)",
	R"(
		class A {
			public void test(A a)
			{
				a.test;
			}
		}
	)",
	R"(
		class A {
			public int foo;
			public void test()
			{
				null.foo;
			}
		}
	)",
	R"(
		class A {
			public void test()
			{
				int foo;
				{
					int foo;
				}
			}
		}
	)",
	R"(
		class A {
			public void test()
			{
				{
					int foo;
				}
				foo;
			}
		}
	)",
	R"(
		class A {
			public boolean foo;
			public void test()
			{
				int foo;
				if(foo)
				{

				}
			}
		}
	)",
	R"(
		class A {
			public int foo;
			public void test()
			{
				if(foo)
				{

				}
			}
		}
	)",
	R"(
		class A {
			public int foo;
			public void test()
			{
				while(foo)
				{

				}
			}
		}
	)",
	R"(
		class A {
			public void test(int foo)
			{
				foo[0];
			}
		}
	)",
	R"(
		class A {
			public void test(int[] foo)
			{
				foo[0][0];
			}
		}
	)",
	R"(
		class A {
			public void test(int[] foo)
			{
				foo[true];
			}
		}
	)",
	R"(
		class A {
			public void test(int[] foo)
			{
				A a;
				foo[a];
			}
		}
	)",
	R"(
		class A {
			public int lol;
			public void test(A[][] foo)
			{
				foo[0].lol;
			}
		}
	)",
	R"(
		class A {
			public void test(int foo)
			{
				!foo;
			}
		}
	)",
	R"(
		class A {
			public void test(A foo)
			{
				!foo;
			}
		}
	)",
	R"(
		class A {
			public void test(int foo)
			{
				!-foo;
			}
		}
	)",
	R"(
		class A {
			public void test(boolean foo)
			{
				-foo;
			}
		}
	)",
	R"(
		class A {
			public void test(A foo)
			{
				-foo;
			}
		}
	)",
	R"(
		class A {
			public void test(boolean foo)
			{
				-!foo;
			}
		}
	)",
	R"(
		class A {
			public void test(boolean foo)
			{
				foo = foo == 3;
			}
		}
	)",
	R"(
		class A {
			public void test(boolean foo)
			{
				foo = 3 + 3;
			}
		}
	)",
	R"(
		class A {
			public void test(A foo)
			{
				foo = 3 - 9;
			}
		}
	)",
	R"(
		class A {
			public void test(A foo)
			{
				return foo;
			}
		}
	)",
	R"(
		class A {
			public int test(A foo)
			{
				return;
			}
		}
	)",
	R"(
		class A {
			public int test(A foo)
			{
				return foo;
			}
		}
	)",
	R"(
		class A {
			public int test()
			{
				foo();
			}
		}
	)",
	R"(
		class A {
			public int test()
			{
				test(3);
			}
		}
	)",
	R"(
		class A {
			public int test(boolean a)
			{
				test(3);
			}
		}
	)",
	R"(
		class A {
			public int test(int a)
			{
				test(true);
			}
		}
	)",
	R"(
		class A {
			public int test(boolean a)
			{
				test(true, 3);
			}
		}
	)",
	R"(
		class A {
			public boolean test()
			{
				int x = test();
			}
		}
	)",
	R"(
		class A {
			public boolean test()
			{
				System.out.println(true);
			}
		}
	)",
	R"(
		class A {
			public boolean test()
			{
				System.out.println();
			}
		}
	)",
	R"(
		class A {
			public boolean test()
			{
				System.out.println(2, 2);
			}
		}
	)",
	R"(
		class A {
			public boolean test()
			{
				System.in;
			}
		}
	)",
	R"(
		class A {
			public boolean test()
			{
				System.out.test();
			}
		}
	)",
};

BOOST_DATA_TEST_CASE(symbol_type_analysis_rejects_invalid_programs, failure_data)
{
	auto pool = minijava::symbol_pool<>{};
	auto lex = make_lexer(std::begin(sample), std::end(sample), pool, pool);
	const auto tokfirst = minijava::token_begin(lex);
	const auto toklast = minijava::token_end(lex);

	auto ast = minijava::parse_program(tokfirst, toklast);

	minijava::semantic::def_annotations def_a{};
	auto typesystem = minijava::semantic::extract_typesystem(*ast, def_a);
	auto system = buildins::register_system(typesystem, pool);
	try{
		minijava::semantic::analyse_program(*ast, {{pool.normalize("System"), system}}, typesystem, def_a);
		TESTAUX_FAIL_NO_EXCEPTION();
	} catch (const minijava::semantic_error& e) {
	}
}
