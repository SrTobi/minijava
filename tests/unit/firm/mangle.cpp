#include "firm/mangle.hpp"

#include <algorithm>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>

#define BOOST_TEST_MODULE  firm_mangle
#include <boost/test/unit_test.hpp>

#include "libfirm/ident.h"

#include "symbol/symbol_pool.hpp"
#include "firm/firm.hpp"


// TODO: Figure out why this "RAII" type doesn't actually RAII.
static minijava::firm_ir firm{};


BOOST_AUTO_TEST_CASE(mangled_format_looks_as_expected)
{
	using namespace std::string_literals;
	auto pool = minijava::symbol_pool<>{};
	const auto classname = pool.normalize("MyClass");
	const auto membername = pool.normalize("myMember");
	//minijava::firm_ir firm{};
	const auto mangled = minijava::firm::mangle(firm, classname, membername);
	const auto expected = "MyClass.myMember"s;
	const auto actual = std::string{get_id_str(mangled)};
	BOOST_REQUIRE_EQUAL(expected, actual);
}


BOOST_AUTO_TEST_CASE(different_names_not_equal)
{
	const auto inputs = std::vector<std::pair<std::string, std::string>>{
		{"Alpha", "alpha"},
		{"Alpha", "beta"},
		{"Beta",  "alpha"},
		{"Beta",  "beta"},
	};
	auto pool = minijava::symbol_pool<>{};
	//minijava::firm_ir firm{};
	auto mangled = std::set<ident*>{};
	std::transform(
		std::begin(inputs), std::end(inputs),
		std::inserter(mangled, mangled.end()),
		[&pool](auto&& pair){
			const auto classname = pool.normalize(pair.first);
			const auto membername = pool.normalize(pair.second);
			return minijava::firm::mangle(firm, classname, membername);
		}
	);
	BOOST_REQUIRE_EQUAL(inputs.size(), mangled.size());
}


BOOST_AUTO_TEST_CASE(same_names_equal)
{
	auto pool = minijava::symbol_pool<>{};
	//minijava::firm_ir firm{};
	const auto classname = pool.normalize("MyClass");
	const auto membername = pool.normalize("member");
	const auto mangled1st = minijava::firm::mangle(firm, classname, membername);
	const auto mangled2nd = minijava::firm::mangle(firm, classname, membername);
	BOOST_REQUIRE(mangled1st == mangled2nd);
}
