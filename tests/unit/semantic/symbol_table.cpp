#include "semantic/symbol_table.hpp"

#include <random>
#include <string>

#define BOOST_TEST_MODULE  semantic_symbol_table
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "semantic/semantic_error.hpp"
#include "symbol/symbol_pool.hpp"
#include "testaux/random_tokens.hpp"

namespace ast = minijava::ast;


namespace /* anonymous */
{

	const void* voided(const void* p) noexcept
	{
		return p;
	}

	std::unique_ptr<ast::var_decl>
	make_decl(minijava::symbol_pool<>& pool, const std::string& name)
	{
		static auto engine = std::default_random_engine{};
		const auto type = testaux::get_random_identifier(engine);
		const auto id = pool.normalize(name);
		const auto tp = pool.normalize(type);
		return std::make_unique<ast::var_decl>(std::make_unique<ast::type>(tp, 333), id);
	}

}


BOOST_AUTO_TEST_CASE(can_create_empty_symbol_table)
{
	const auto st = minijava::sem::symbol_table{};
	BOOST_REQUIRE_EQUAL(0, st.depth());
}


BOOST_AUTO_TEST_CASE(initial_scope_contains_nothing)
{
	const std::string strings[] = {
		"", "if", "again", "and", "again", "System", "String", "foo",
		"System.out.println", "main", "args", "0", "1234", "!", "@#!?",
	};
	auto pool = minijava::symbol_pool<>{};
	auto st = minijava::sem::symbol_table{};
	st.enter_scope();
	BOOST_REQUIRE(std::none_of(
		std::begin(strings), std::end(strings),
		[&](auto&& s){ return bool(st.lookup(pool.normalize(s))); }
	));
	BOOST_REQUIRE(std::none_of(
		std::begin(strings), std::end(strings),
		[&](auto&& s){ return bool(st.get_conflicting_definitions(pool.normalize(s))); }
	));
}


BOOST_AUTO_TEST_CASE(can_enter_and_leave_scopes_ad_libitum)
{
	auto engine = std::default_random_engine{};
	auto dist = std::bernoulli_distribution{};
	auto st = minijava::sem::symbol_table{};
	auto expected = std::size_t{0};
	for (auto i = 0; i < 1000; ++i) {
		if ((expected == 0) || dist(engine)) {
			st.enter_scope();
			++expected;
		} else {
			st.leave_scope();
			--expected;
		}
		BOOST_REQUIRE_EQUAL(expected, st.depth());
	}
}


BOOST_AUTO_TEST_CASE(contains_definition_after_adding)
{
	auto pool = minijava::symbol_pool<>{};
	auto st = minijava::sem::symbol_table{};
	st.enter_scope();
	const auto dcl = make_decl(pool, "alpha");
	st.add_def(dcl->name(), dcl.get());
	BOOST_REQUIRE_EQUAL(dcl.get(), st.lookup(dcl->name()).get());
}


BOOST_AUTO_TEST_CASE(forgets_definition_at_scope_exit)
{
	auto pool = minijava::symbol_pool<>{};
	auto st = minijava::sem::symbol_table{};
	const auto dcl1st = make_decl(pool, "alpha");
	const auto dcl2nd = make_decl(pool, "beta");
	st.enter_scope();
	BOOST_REQUIRE(!bool(st.lookup(dcl1st->name())));
	BOOST_REQUIRE(!bool(st.lookup(dcl2nd->name())));
	st.add_def(dcl1st->name(), dcl1st.get());
	BOOST_REQUIRE_EQUAL(dcl1st.get(), st.lookup(dcl1st->name()).get());
	BOOST_REQUIRE(!bool(st.lookup(dcl2nd->name())));
	st.enter_scope();
	BOOST_REQUIRE_EQUAL(dcl1st.get(), st.lookup(dcl1st->name()).get());
	BOOST_REQUIRE(!bool(st.lookup(dcl2nd->name())));
	st.add_def(dcl2nd->name(), dcl2nd.get());
	BOOST_REQUIRE_EQUAL(dcl1st.get(), st.lookup(dcl1st->name()).get());
	BOOST_REQUIRE_EQUAL(dcl2nd.get(), st.lookup(dcl2nd->name()).get());
	st.leave_scope();
	BOOST_REQUIRE_EQUAL(dcl1st.get(), st.lookup(dcl1st->name()).get());
	BOOST_REQUIRE(!bool(st.lookup(dcl2nd->name())));
}


BOOST_AUTO_TEST_CASE(accepts_nullptr)
{
	auto pool = minijava::symbol_pool<>{};
	auto st = minijava::sem::symbol_table{};
	const auto p = pool.normalize("p");
	const auto q = pool.normalize("q");
	st.enter_scope();
	st.add_def(p, nullptr);
	st.add_def(q, nullptr);
	BOOST_REQUIRE_EQUAL(voided(nullptr), voided(st.lookup(p).get()));
	BOOST_REQUIRE_EQUAL(voided(nullptr), voided(st.lookup(q).get()));
}


static const bool all_bools[] = {false, true};

BOOST_DATA_TEST_CASE(always_rejects_same_name_twice_in_scope, all_bools)
{
	auto pool = minijava::symbol_pool<>{};
	auto st = minijava::sem::symbol_table{};
	const auto def1st = make_decl(pool, "alpha");
	const auto def2nd = make_decl(pool, "alpha");
	st.enter_scope(sample);
	st.add_def(def1st->name(), def1st.get());
	BOOST_REQUIRE_THROW(
		st.add_def(def2nd->name(), def2nd.get()),
		minijava::semantic_error
	);
}


BOOST_AUTO_TEST_CASE(shadowing_allowed)
{
	auto pool = minijava::symbol_pool<>{};
	auto st = minijava::sem::symbol_table{};
	const auto a1 = make_decl(pool, "alpha");
	const auto a2 = make_decl(pool, "alpha");
	const auto b = make_decl(pool, "beta");
	st.enter_scope(true);
	st.add_def(b->name(), b.get());
	st.add_def(a1->name(), a1.get());
	st.enter_scope();
	BOOST_REQUIRE_EQUAL(a1.get(), st.lookup(a1->name()).get());
	BOOST_REQUIRE_EQUAL(a1.get(), st.lookup(a2->name()).get());
	BOOST_REQUIRE_EQUAL(b.get(), st.lookup(b->name()).get());
	st.add_def(a2->name(), a2.get());
	BOOST_REQUIRE_EQUAL(a2.get(), st.lookup(a1->name()).get());
	BOOST_REQUIRE_EQUAL(a2.get(), st.lookup(a2->name()).get());
	BOOST_REQUIRE_EQUAL(b.get(), st.lookup(b->name()).get());
	st.leave_scope();
	BOOST_REQUIRE_EQUAL(a1.get(), st.lookup(a1->name()).get());
	BOOST_REQUIRE_EQUAL(a1.get(), st.lookup(a2->name()).get());
	BOOST_REQUIRE_EQUAL(b.get(), st.lookup(b->name()).get());
}


BOOST_AUTO_TEST_CASE(shadowing_disallowed)
{
	auto pool = minijava::symbol_pool<>{};
	auto st = minijava::sem::symbol_table{};
	const auto a1 = make_decl(pool, "alpha");
	const auto a2 = make_decl(pool, "alpha");
	st.enter_scope();
	st.add_def(a1->name(), a1.get());
	st.enter_scope();
	BOOST_REQUIRE_THROW(
		st.add_def(a2->name(), a2.get()),
		minijava::semantic_error
	);
	BOOST_REQUIRE_EQUAL(a1.get(), st.get_conflicting_definitions(a1->name()).get());
}
