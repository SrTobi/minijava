#include "parser/parser.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#define BOOST_TEST_MODULE  parser_parser
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "lexer/token.hpp"
#include "lexer/token_type.hpp"
#include "parser/ast.hpp"
#include "parser/pretty_printer.hpp"
#include "symbol_pool.hpp"

#include "testaux/testaux.hpp"
#include "testaux/token_string.hpp"


namespace ast = minijava::ast;


// We provide some convenience macros to create token sequences from
// higher-level abstractions.  They loosely model an AST of MiniJava and take
// as parameters the respective sub-trees.  Don't expect anything smart here.
// These were hacked together to get a job done.

// This macro is needed to protect macro arguments from premature expansion.
#define PROTECT(...)  __VA_ARGS__

#define PDE(Token)                                                            \
	pde(), Token

#define PARAMS(...)                                                           \
	tt::left_paren, __VA_ARGS__, tt::right_paren

#define EMPTY_PARAMS                                                          \
	tt::left_paren, tt::right_paren

#define BLOCK(...)                                                            \
	tt::left_brace, __VA_ARGS__, tt::right_brace

#define EMPTY_BLOCK                                                           \
	tt::left_brace, tt::right_brace

#define ARRAY(Type)                                                           \
	Type, tt::left_bracket, tt::right_bracket

#define STMT(...)                                                             \
	__VA_ARGS__, tt::semicolon

#define FIELD(Type, Name)                                                     \
	STMT(tt::kw_public, PROTECT(Type), id(Name))

#define CLASS(Name, Body)                                                     \
	tt::kw_class, id(Name), Body

#define MAIN_METHOD_TYPED(RetType, Name, Args, Body)                          \
	tt::kw_public, tt::kw_static, RetType, id(Name), Args, Body

#define MAIN_METHOD(Name, ArgName, Body)                                      \
	MAIN_METHOD_TYPED(tt::kw_void, Name, PARAMS(ARRAY(id("String")), id(ArgName)), PROTECT(Body))

#define METHOD(RetType, Name, Args, Body)                                     \
	tt::kw_public, RetType, id(Name), Args, Body

#define PROGRAM(...)                                                          \
	CLASS("Foo", BLOCK(METHOD(tt::kw_int, "bar", EMPTY_PARAMS, BLOCK(__VA_ARGS__))))

#define IF(Cond, Then)                                                        \
	tt::kw_if, tt::left_paren, Cond, tt::right_paren, Then

#define IFELSE(Cond, Then, Else)                                              \
	IF(PROTECT(Cond), PROTECT(Then)), tt::kw_else, Else

#define WHILE(Cond, Body)                                                     \
	tt::kw_while, tt::left_paren, Cond, tt::right_paren, Body

#define RETURN(Expr)                                                          \
	STMT(tt::kw_return, Expr)

#define ARRAY_SUB(Array, ...)                                                 \
	Array, tt::left_bracket, __VA_ARGS__, tt::right_bracket

#define CALL(Func, Params)                                                    \
	id(Func), Params

#define PAREN(...)                                                            \
	tt::left_paren, __VA_ARGS__, tt::right_paren

#define NEW_OBJ(Type)                                                         \
	tt::kw_new, CALL(Type, EMPTY_PARAMS)

#define NEW_ARRAY(Type, ...)                                                  \
	tt::kw_new, Type, tt::left_bracket, __VA_ARGS__, tt::right_bracket


using testaux::id;
using testaux::lit;
using tt = minijava::token_type;


namespace /* anonymous */
{

	// The `token_sequence` helper `class` eases the creation of test cases.
	// Its constructor takes any number of `token_type`s and `tagged_string`s
	// to construct a token sequence as parser input.  If the arguments contain
	// the EOF token, its position is used as the position of the
	// parser-defined error location and the token is removed afterwards.  The
	// column number of each token will be set to an incrementing sequence and
	// the location of the PDE is available via the `pde_idx` member function.
	// The token sequence itself is exposed via the `begin` and `end` member
	// functions.

    static minijava::symbol_pool<> g_pool{};

	class token_sequence
	{
	public:

		template <typename... ArgTs>
		token_sequence(ArgTs&&... args)
		{
			const auto eof = minijava::token::create(tt::eof);
			_tokens = {testaux::make_token(g_pool, std::forward<ArgTs>(args))...};
			// An EOF token is an indicator that the following token is the PDE.
			const auto pos = std::find(_tokens.begin(), _tokens.end(), eof);
			if (pos != _tokens.end()) {
				_pde_index = static_cast<std::size_t>(std::distance(_tokens.begin(), pos)) + 1;
				_tokens.erase(pos);
			}
			_tokens.push_back(eof);
			for(std::size_t i = 0; i < _tokens.size(); ++i) {
				_tokens[i].set_column(i + 1);
			}
		}

		auto begin() const noexcept
		{
			return _tokens.begin();
		}

		auto end() const noexcept
		{
			return _tokens.end();
		}

		auto pde_idx() const noexcept
		{
			return _pde_index;
		}

	private:

		std::size_t _pde_index{};

		std::vector<minijava::token> _tokens{};

		friend std::ostream& operator<<(std::ostream& os, const token_sequence& ts)
		{
			auto stringify = [](auto&& t) -> std::string {
				return t.has_lexval() ? t.lexval().c_str() : name(t.type());
			};
			std::transform(ts.begin(), ts.end(),
			               std::ostream_iterator<std::string>(os, " "),
			               stringify);
			return os;
		}
	};

	tt pde()
	{
		return tt::eof;
	}

}  // namespace /* anonymous */


static const token_sequence success_data[] = {
	{},
	{CLASS("Foo", EMPTY_BLOCK)},
	{CLASS("Foo", BLOCK(FIELD(tt::kw_int, "member")))},
	{CLASS("Foo", BLOCK(MAIN_METHOD("main", "args", EMPTY_BLOCK)))},
	{
		CLASS("Foo", BLOCK(
			METHOD(tt::kw_int, "bar", EMPTY_PARAMS, EMPTY_BLOCK))
		)
	},
	{
		CLASS("Foo", BLOCK(
			METHOD(tt::kw_int, "bar", PARAMS(tt::kw_int, id("x")), EMPTY_BLOCK)
			)
		)
	},
	{
		CLASS("Foo", BLOCK(
			METHOD(tt::kw_int, "bar",
			       PARAMS(tt::kw_int, id("x"), tt::comma, tt::kw_int, id("y")),
				   EMPTY_BLOCK)
			)
		)
	},
	{
		CLASS("Foo", BLOCK(
			METHOD(ARRAY(tt::kw_int), "bar",
			       PARAMS(ARRAY(tt::kw_int), id("x")),
				   EMPTY_BLOCK)
			)
		)
	},
	{CLASS("Foo", BLOCK(FIELD(ARRAY(ARRAY(ARRAY(tt::kw_int))), "array")))},
	{
		CLASS("Foo", EMPTY_BLOCK),
		CLASS("Bar", BLOCK(
			METHOD(tt::kw_int, "inBar1", PARAMS(id("Foo"), id("foo")), EMPTY_BLOCK),
			METHOD(id("Foo"), "inBar2", EMPTY_PARAMS, EMPTY_BLOCK),
			FIELD(tt::kw_boolean, "myBool"),
			MAIN_METHOD("main1", "args", EMPTY_BLOCK),
			MAIN_METHOD("main2", "args", EMPTY_BLOCK)
		))
	},
	{PROGRAM(tt::semicolon)},
	{PROGRAM(EMPTY_BLOCK)},
	{PROGRAM(IF(lit("1"), EMPTY_BLOCK))},
	{PROGRAM(IFELSE(lit("1"), EMPTY_BLOCK, EMPTY_BLOCK))},
	{PROGRAM(WHILE(lit("1"), EMPTY_BLOCK))},
	{PROGRAM(WHILE(lit("1"), EMPTY_BLOCK))},
	{PROGRAM(RETURN(lit("11")))},
	{PROGRAM(STMT(tt::kw_return))},
	{PROGRAM(STMT(tt::kw_return, tt::kw_null))},
	{PROGRAM(STMT(tt::kw_return, tt::kw_this))},
	{PROGRAM(STMT(tt::kw_return, tt::kw_false))},
	{PROGRAM(STMT(tt::kw_return, tt::kw_true))},
	{PROGRAM(STMT(tt::kw_return, id("foo")))},
	{PROGRAM(STMT(tt::kw_return), STMT(tt::kw_return))},
	{PROGRAM(STMT(lit("1"), tt::plus, id("itegerlit")))},
	{PROGRAM(STMT(ARRAY_SUB(id("args"), lit("0"))))},
	{PROGRAM(STMT(id("args"), tt::dot, id("length")))},
	{PROGRAM(STMT(lit("0"), tt::dot, id("property")))},
	{PROGRAM(STMT(PAREN(lit("0"), tt::plus, tt::kw_false)))},
	{PROGRAM(STMT(CALL("crash", EMPTY_PARAMS)))},
	{PROGRAM(STMT(CALL("fib", PARAMS(lit("3237834374672643")))))},
	{PROGRAM(STMT(CALL("gcd", PARAMS(id("p"), tt::comma,id("q")))))},
	{PROGRAM(STMT(NEW_OBJ("Foo")))},
	{PROGRAM(STMT(NEW_ARRAY(id("Foo"), lit("1"), tt::plus, lit("2"))))},
	{PROGRAM(STMT(ARRAY(NEW_ARRAY(id("Foo"), lit("1"), tt::plus, lit("2")))))},
	{PROGRAM(STMT(ARRAY(NEW_ARRAY(tt::kw_void, lit("1"), tt::plus, lit("2")))))},
	{PROGRAM(STMT(ARRAY_SUB(ARRAY(NEW_ARRAY(tt::kw_void, lit("1"))), id("muhaha"))))},
	{PROGRAM(STMT(id("foo"), tt::assign, lit("3"), tt::minus, lit("5"), tt::multiply, lit("5")))},
	{PROGRAM(STMT(lit("3"), tt::minus, tt::minus, tt::minus, lit("5")))},
	{PROGRAM(STMT(id("foo"), tt::assign, id("bar")))},
	{PROGRAM(STMT(tt::kw_int, id("foo")))},
	{PROGRAM(STMT(id("Foo"), id("foo")))},
	{PROGRAM(STMT(ARRAY(id("Foo")), id("arr")))},
	{PROGRAM(STMT(ARRAY(ARRAY(id("Foo"))), id("arr")))},
	{PROGRAM(STMT(ARRAY(ARRAY(tt::kw_int)), id("arr")))},
	{
		PROGRAM(
			STMT(
				ARRAY_SUB(
					ARRAY_SUB(id("arr"), lit("9")), id("i"), tt::modulo, lit("0")
				),
				tt::assign, lit("5")
			)
		)
	},
};

BOOST_DATA_TEST_CASE(parser_accepts_valid_programs, success_data)
{
	assert(sample.pde_idx() == 0);
	try{
		minijava::parse_program(std::begin(sample), std::end(sample));
	}catch(const minijava::syntax_error& e)
	{
		BOOST_FAIL("Exception thrown: " << e.what());
	}
}


static const token_sequence failure_data[] = {
	{pde(), tt::semicolon},
	{tt::kw_class, PDE(lit("5"))},
	{tt::kw_class, PDE(tt::kw_class)},
	{tt::kw_class, id("Foo1"), PDE(tt::kw_class), id("Foo2")},
	{
		CLASS("Foo", BLOCK(
			MAIN_METHOD_TYPED(
				PDE(tt::kw_int), "main",
				PARAMS(ARRAY(id("String")), id("args")),
				EMPTY_BLOCK
			)
		))
	},
	{
		CLASS("Foo", BLOCK(
			MAIN_METHOD_TYPED(
				tt::kw_void, "main",
				PARAMS(ARRAY(PDE(tt::kw_int)), id("args")),
				EMPTY_BLOCK
			)
		))
	},
	{
		CLASS("Foo", BLOCK(
			MAIN_METHOD_TYPED(
				tt::kw_void, "main",
				PARAMS(ARRAY(PDE(id("SomeType"))), id("args")),
				EMPTY_BLOCK
			)
		))
	},
	{
		CLASS("Foo", BLOCK(
			MAIN_METHOD_TYPED(
				tt::kw_void, "main",
				PARAMS(id("String"), PDE(id("args"))),
				EMPTY_BLOCK
			)
		))
	},
	{
		CLASS("Foo", BLOCK(
			METHOD(
				tt::kw_int, "bar",
				PARAMS(tt::kw_int, id("x"), PDE(tt::kw_int), id("y")),
				EMPTY_BLOCK
			)
		))
	},
	{
		CLASS("Foo", BLOCK(
			METHOD(
				tt::kw_int, "bar",
				PARAMS(tt::kw_int, id("x"), tt::comma, pde()),
				EMPTY_BLOCK
			)
		))
	},
	{
		CLASS("Foo", BLOCK(
			METHOD(
				tt::kw_int, "bar",
				PARAMS(PDE(tt::comma), tt::kw_int, id("x")),
				EMPTY_BLOCK
			)
		))
	},
	{
		CLASS("Foo", BLOCK(
			STMT(tt::kw_public, ARRAY(tt::kw_int), tt::left_bracket, PDE(id("array")))
		))
	},
	{
		CLASS("Foo", BLOCK(
			STMT(tt::kw_public, tt::kw_int, PDE(tt::right_bracket), id("array"))
		))
	},
	{PROGRAM(tt::kw_if, tt::left_paren, PDE(tt::right_paren), tt::semicolon)},
	{PROGRAM(IF(tt::kw_true, STMT(PDE(tt::kw_int), id("avar"))))},
	{PROGRAM(STMT(tt::kw_return, PDE(tt::kw_return)))},
	{PROGRAM(STMT(tt::kw_return, lit("5"), PDE(tt::comma), lit("4")))},
	{PROGRAM(STMT(tt::kw_return, tt::left_paren, PDE(tt::right_paren)))},
	{PROGRAM(STMT(id("args"), tt::dot, PDE(lit("0"))))},
	{PROGRAM(STMT(CALL("gcd", PARAMS(id("p"), PDE(id("q"))))))},
	{PROGRAM(STMT(tt::kw_new, tt::kw_int, pde(), EMPTY_PARAMS))},
	{PROGRAM(STMT(id("b"), tt::less_than, PDE(tt::greater_than), id("d")))},
	{PROGRAM(STMT(tt::kw_int, PDE(lit("5"))))},
	{PROGRAM(STMT(ARRAY_SUB(id("arr"), lit("9")), tt::left_bracket, PDE(tt::right_bracket)))},
	{PROGRAM(STMT(ARRAY_SUB(ARRAY(id("Foo")), PDE(lit("9")))))},
};

BOOST_DATA_TEST_CASE(parser_rejects_invalid_programs, failure_data)
{
	const auto pde_idx = sample.pde_idx();
	assert(pde_idx > 0);

	try {
		minijava::parse_program(std::begin(sample), std::end(sample));
		TESTAUX_FAIL_NO_EXCEPTION();
	} catch (const minijava::syntax_error& e) {
		if (pde_idx != e.column()) {
			std::clog << "Caught exception: " << e.what() << std::endl;
		}
		BOOST_REQUIRE_EQUAL(pde_idx, e.column());
	}
}


namespace /* anonymous */
{

	template <minijava::token_type... TTs>
	minijava::syntax_error get_thrown_syntax_error(const minijava::token& tok)
	{
		try {
			minijava::detail::throw_syntax_error(tok, {TTs...});
			throw std::runtime_error{__func__};
		} catch (const minijava::syntax_error& e) {
			return e;
		}
	}

}


BOOST_AUTO_TEST_CASE(throw_syntax_error_correct_source_location)
{
	auto tok = minijava::token::create(tt::semicolon);
	tok.set_line(1234);
	tok.set_column(56);
	const auto e = get_thrown_syntax_error<tt::eof>(tok);
	BOOST_REQUIRE_EQUAL(tok.line(), e.line());
	BOOST_REQUIRE_EQUAL(tok.column(), e.column());
}


BOOST_AUTO_TEST_CASE(throw_syntax_error_single_expected_token_1st)
{
	using namespace std::string_literals;
	const auto tok = minijava::token::create(tt::kw_if);
	const auto e = get_thrown_syntax_error<tt::identifier>(tok);
	BOOST_REQUIRE_EQUAL("Expected identifier but found keyword 'if'"s, e.what());
}


BOOST_AUTO_TEST_CASE(throw_syntax_error_single_expected_token_2nd)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	const auto tok = minijava::token::create_identifier(pool.normalize("foo"));
	const auto e = get_thrown_syntax_error<tt::kw_if>(tok);
	BOOST_REQUIRE_EQUAL("Expected keyword 'if' but found identifier 'foo'"s, e.what());
}


BOOST_AUTO_TEST_CASE(throw_syntax_error_two_expected_tokens)
{
	using namespace std::string_literals;
	const auto tok = minijava::token::create(tt::colon);
	const auto e = get_thrown_syntax_error<tt::plus, tt::minus>(tok);
	BOOST_REQUIRE_EQUAL("Expected '+' or '-' but found ':'"s, e.what());
}


BOOST_AUTO_TEST_CASE(throw_syntax_error_three_expected_tokens)
{
	using namespace std::string_literals;
	const auto tok = minijava::token::create(tt::eof);
	const auto e = get_thrown_syntax_error<tt::kw_for, tt::kw_do, tt::kw_while>(tok);
	BOOST_REQUIRE_EQUAL(
		"Expected keyword 'for', keyword 'do' or keyword 'while' but found EOF"s,
		e.what()
	);
}


namespace /* anonymous */
{
	template <typename T>
	std::enable_if_t<std::is_base_of<ast::node, T>{} && std::is_final<T>{}, std::string>
	serialize(T& ast_node)
	{
		std::ostringstream oss {};
		auto pp = ast::pretty_printer{oss};
		pp.visit(ast_node);
		return oss.str();
	}
}


BOOST_AUTO_TEST_CASE(ast_empty_program)
{
	const token_sequence test_data{};
	auto expected_ast = std::make_unique<ast::program>();
	auto actual_ast = minijava::parse_program(std::begin(test_data), std::end(test_data));
	BOOST_REQUIRE_EQUAL(serialize(*expected_ast), serialize(*actual_ast));
}


BOOST_AUTO_TEST_CASE(ast_single_empty_class)
{
	const token_sequence test_data{CLASS("Example", EMPTY_BLOCK)};
	auto pool = minijava::symbol_pool<>{};
	auto expected_ast = std::make_unique<ast::program>();
	expected_ast->add_class(std::make_unique<ast::class_declaration>(pool.normalize("Example")));
	auto actual_ast = minijava::parse_program(std::begin(test_data), std::end(test_data));
	BOOST_REQUIRE_EQUAL(serialize(*expected_ast), serialize(*actual_ast));
}


BOOST_AUTO_TEST_CASE(ast_multiple_empty_classes)
{
	using namespace std::string_literals;
	const token_sequence test_data{
		CLASS("Alpha", EMPTY_BLOCK),
		CLASS("Beta",  EMPTY_BLOCK),
		CLASS("Gamma", EMPTY_BLOCK),
		CLASS("Delta", EMPTY_BLOCK),
	};
	auto pool = minijava::symbol_pool<>{};
	auto expected_ast = std::make_unique<ast::program>();
	for (const auto& name : {"Alpha"s, "Beta"s, "Gamma"s, "Delta"s}) {
		const auto canon = pool.normalize(name);
		expected_ast->add_class(std::make_unique<ast::class_declaration>(canon));
	}
	auto actual_ast = minijava::parse_program(std::begin(test_data), std::end(test_data));
	BOOST_REQUIRE_EQUAL(serialize(*expected_ast), serialize(*actual_ast));
}


BOOST_AUTO_TEST_CASE(ast_class_with_fields)
{
	const token_sequence test_data{
		CLASS("Employee",
			BLOCK(
				FIELD(id("Text"), "name"),
				FIELD(tt::kw_int, "sallery"),
				FIELD(tt::kw_boolean, "trustworthy"),
				FIELD(tt::kw_void, "questionable"),
				FIELD(ARRAY(tt::kw_void), "questionable"),
				FIELD(ARRAY(ARRAY(tt::kw_void)), "questionable"),
				FIELD(ARRAY(ARRAY(ARRAY(tt::kw_void))), "questionable")
			)
		)
	};
	auto pool = minijava::symbol_pool<>{};
	auto expected_ast = std::make_unique<ast::program>();
	if (auto cls = std::make_unique<ast::class_declaration>(pool.normalize("Employee"))) {
		{
			const auto id = pool.normalize("name");
			const auto tpnam = pool.normalize("Text");
			auto typ = std::make_unique<ast::type>(tpnam);
			auto dcl = std::make_unique<ast::var_decl>(std::move(typ), id);
			cls->add_field(std::move(dcl));
		}
		{
			const auto id = pool.normalize("sallery");
			auto typ = std::make_unique<ast::type>(ast::primitive_type::type_int);
			auto dcl = std::make_unique<ast::var_decl>(std::move(typ), id);
			cls->add_field(std::move(dcl));
		}
		{
			const auto id = pool.normalize("trustworthy");
			auto typ = std::make_unique<ast::type>(ast::primitive_type::type_boolean);
			auto dcl = std::make_unique<ast::var_decl>(std::move(typ), id);
			cls->add_field(std::move(dcl));
		}
		for (auto rank = std::size_t{}; rank <= 3; ++rank) {
			const auto id = pool.normalize("questionable");
			auto typ = std::make_unique<ast::type>(ast::primitive_type::type_void, rank);
			auto dcl = std::make_unique<ast::var_decl>(std::move(typ), id);
			cls->add_field(std::move(dcl));
		}
		expected_ast->add_class(std::move(cls));
	}
	auto actual_ast = minijava::parse_program(std::begin(test_data), std::end(test_data));
	BOOST_REQUIRE_EQUAL(serialize(*expected_ast), serialize(*actual_ast));
}


BOOST_AUTO_TEST_CASE(ast_methods_primitive)
{
	using namespace std::string_literals;
	const token_sequence test_data{
		CLASS("Example",
			BLOCK(
				METHOD(tt::kw_int, "nullary", EMPTY_PARAMS, EMPTY_BLOCK),
				METHOD(tt::kw_int, "unary", PARAMS(
					tt::kw_int, id("a")
				), EMPTY_BLOCK),
				METHOD(tt::kw_int, "binary", PARAMS(
					tt::kw_int, id("a"), tt::comma,
					tt::kw_int, id("b")
				), EMPTY_BLOCK),
				METHOD(tt::kw_int, "ternary", PARAMS(
					tt::kw_int, id("a"), tt::comma,
					tt::kw_int, id("b"), tt::comma,
					tt::kw_int, id("c")
				), EMPTY_BLOCK)
			)
		)
	};
	auto pool = minijava::symbol_pool<>{};
	auto expected_ast = std::make_unique<ast::program>();
	if (auto cls = std::make_unique<ast::class_declaration>(pool.normalize("Example"))) {
		const std::pair<std::string, std::size_t> methods[] = {
			{"nullary", 0},
			{"unary",   1},
			{"binary",  2},
			{"ternary", 3},
		};
		const std::string parameters[] = {"a", "b", "c"};
		for (const auto& method : methods) {
			const auto id = pool.normalize(method.first);
			auto typ = std::make_unique<ast::type>(ast::primitive_type::type_int);
			auto prm = std::vector<std::unique_ptr<ast::var_decl>>{};
			for (auto i = std::size_t{}; i < method.second; ++i) {
				const auto prmnam = pool.normalize(parameters[i]);
				auto prmtyp = std::make_unique<ast::type>(ast::primitive_type::type_int);
				auto prmdcl = std::make_unique<ast::var_decl>(std::move(prmtyp), prmnam);
				prm.push_back(std::move(prmdcl));
			}
			auto blk = std::make_unique<ast::block>();
			auto mtd = std::make_unique<ast::method>(
				id, std::move(typ), std::move(prm), std::move(blk)
			);
			cls->add_method(std::move(mtd));
		}
		expected_ast->add_class(std::move(cls));
	}
	auto actual_ast = minijava::parse_program(std::begin(test_data), std::end(test_data));
	BOOST_REQUIRE_EQUAL(serialize(*expected_ast), serialize(*actual_ast));
}


BOOST_AUTO_TEST_CASE(ast_methods_udt)
{
	using namespace std::string_literals;
	const token_sequence test_data{
		CLASS("Example",
			BLOCK(
				METHOD(ARRAY(id("Alpha")), "doStuff", PARAMS(
					ARRAY(id("Alpha")), id("a")
				), EMPTY_BLOCK)
			)
		)
	};
	auto pool = minijava::symbol_pool<>{};
	auto expected_ast = std::make_unique<ast::program>();
	if (auto cls = std::make_unique<ast::class_declaration>(pool.normalize("Example"))) {
		const auto id = pool.normalize("doStuff");
		const auto alp = pool.normalize("Alpha");
		auto typ = std::make_unique<ast::type>(alp, 1);
		auto prm = std::vector<std::unique_ptr<ast::var_decl>>{};
		{
			const auto prmnam = pool.normalize("a");
			auto prmtyp = std::make_unique<ast::type>(alp, 1);
			auto prmdcl = std::make_unique<ast::var_decl>(std::move(prmtyp), prmnam);
			prm.push_back(std::move(prmdcl));
		}
		auto blk = std::make_unique<ast::block>();
		auto mtd = std::make_unique<ast::method>(
			id, std::move(typ), std::move(prm), std::move(blk)
		);
		cls->add_method(std::move(mtd));
		expected_ast->add_class(std::move(cls));
	}
	auto actual_ast = minijava::parse_program(std::begin(test_data), std::end(test_data));
	BOOST_REQUIRE_EQUAL(serialize(*expected_ast), serialize(*actual_ast));
}


BOOST_AUTO_TEST_CASE(ast_methods_main)
{
	using namespace std::string_literals;
	const token_sequence test_data{
		CLASS("Example",
			BLOCK(
				MAIN_METHOD("notmain", "args", EMPTY_BLOCK),
				MAIN_METHOD("notmain", "sgra", EMPTY_BLOCK)
			)
		)
	};
	auto pool = minijava::symbol_pool<>{};
	auto expected_ast = std::make_unique<ast::program>();
	if (auto cls = std::make_unique<ast::class_declaration>(pool.normalize("Example"))) {
		const auto id = pool.normalize("notmain");
		for (const auto& arg : {"args"s, "sgra"s}) {
			const auto argid = pool.normalize(arg);
			auto blk = std::make_unique<ast::block>();
			auto mm = std::make_unique<ast::main_method>(id, argid, std::move(blk));
			cls->add_main_method(std::move(mm));
		}
		expected_ast->add_class(std::move(cls));
	}
	auto actual_ast = minijava::parse_program(std::begin(test_data), std::end(test_data));
	BOOST_REQUIRE_EQUAL(serialize(*expected_ast), serialize(*actual_ast));
}
