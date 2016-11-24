#include "semantic/attribute.hpp"

#include <algorithm>
#include <iterator>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>

#define BOOST_TEST_MODULE  semantic_attribute
#include <boost/test/unit_test.hpp>

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"

#include "testaux/testaux.hpp"


namespace ast = minijava::ast;

namespace /* anonymous */
{

	struct not_default_constructible
	{
		explicit not_default_constructible(const int t) : tag{t} {}
		int tag{};
	};

	static_assert(!std::is_default_constructible<not_default_constructible>{}, "");

	const void* voided(const void* p) noexcept
	{
		return p;
	}

	struct dummy_ast_node : minijava::ast::node
	{
		void accept(ast::visitor&) const override {}
	};

	struct mock_filter
	{
		template<template <typename> class TagT, typename T>
		static constexpr bool static_check(TagT<T>) noexcept { return true; }

		template <typename T>
		int dynamic_check(T&&) { return *flagptr; }

		int* flagptr{};
	};

	// template <typename T>
	// struct mock_allocator : std::allocator<T>
	// {
	//  using value_type = T;
	//  mock_allocator() = delete;
	//  mock_allocator(const int me) : tag{me} {}
	//  template <class U> mock_allocator(const mock_allocator<U> other) : tag{other.tag} {}
	//  int tag{};
	// };

	// template <class T1, class T2>
	// bool operator==(const mock_allocator<T1>& lhs, const mock_allocator<T2>& rhs)
	// {
	//  return (lhs.tag == rhs.tag);
	// }

	// template <class T1, class T2>
	// bool operator!=(const mock_allocator<T1>& lhs, const mock_allocator<T2>& rhs)
	// {
	//  return !(lhs == rhs);
	// }

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(default_constructed_map_is_empty_and_has_size_zero)
{
	const auto atmap = minijava::ast_attributes<int>{};
	BOOST_REQUIRE(atmap.empty());
	BOOST_REQUIRE_EQUAL(0, atmap.size());
}


BOOST_AUTO_TEST_CASE(default_filter_accepts_any_node)
{
	auto atmap = minijava::ast_attributes<int>{};
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	BOOST_REQUIRE_NO_THROW(atmap[*nodeptr]);                     // lvalue
	BOOST_REQUIRE_NO_THROW(atmap[testaux::as_const(*nodeptr)]);  // const lvalue
	BOOST_REQUIRE_NO_THROW(atmap[std::move(*nodeptr)]);          // rvalue
}


BOOST_AUTO_TEST_CASE(ctor_with_policies)
{
	const auto m0 = minijava::ast_attributes<int>{};
	const auto m1a = minijava::ast_attributes<int>{m0.get_filter()};
	const auto m1b = minijava::ast_attributes<int>{m0.get_allocator()};
	const auto m2 = minijava::ast_attributes<int>{m0.get_filter(), m0.get_allocator()};
}


BOOST_AUTO_TEST_CASE(max_size_returns_something_large)
{
	const auto atmap = minijava::ast_attributes<int>{};
	BOOST_REQUIRE_GT(atmap.max_size(), std::size_t{1000000});
}


BOOST_AUTO_TEST_CASE(insert_const_lvalue)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<int>{};
	const auto value = decltype(atmap)::value_type{nodeptr.get(), 42};
	const auto ip = atmap.insert(value);
	BOOST_REQUIRE(ip.second);
	BOOST_REQUIRE_EQUAL(voided(nodeptr.get()), voided(ip.first->first));
	BOOST_REQUIRE_EQUAL(42, ip.first->second);
	BOOST_REQUIRE(!atmap.empty());
	BOOST_REQUIRE_EQUAL(1, atmap.size());
}


BOOST_AUTO_TEST_CASE(insert_rvalue)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<std::unique_ptr<int>>{};
	const auto ip = atmap.insert({nodeptr.get(), std::make_unique<int>(42)});
	BOOST_REQUIRE(ip.second);
	BOOST_REQUIRE_EQUAL(voided(nodeptr.get()), voided(ip.first->first));
	BOOST_REQUIRE_EQUAL(42, *(ip.first->second));
	BOOST_REQUIRE(!atmap.empty());
	BOOST_REQUIRE_EQUAL(1, atmap.size());
}


BOOST_AUTO_TEST_CASE(insert_already_present_const_lvalue)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<int>{};
	const auto val1st = decltype(atmap)::value_type{nodeptr.get(), 42};
	const auto val2nd = decltype(atmap)::value_type{nodeptr.get(), 77};
	atmap.insert(val1st);
	const auto ip = atmap.insert(val2nd);
	BOOST_REQUIRE(!ip.second);
	BOOST_REQUIRE(*ip.first == val1st);
	BOOST_REQUIRE_EQUAL(1, atmap.size());
}


BOOST_AUTO_TEST_CASE(insert_already_present_rvalue)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<std::unique_ptr<int>>{};
	atmap.insert({nodeptr.get(), std::make_unique<int>(42)});
	const auto ip = atmap.insert({nodeptr.get(), std::make_unique<int>(7)});
	BOOST_REQUIRE(!ip.second);
	BOOST_REQUIRE_EQUAL(voided(nodeptr.get()), voided(ip.first->first));
	BOOST_REQUIRE_EQUAL(42, *(ip.first->second));
	BOOST_REQUIRE_EQUAL(1, atmap.size());
}


BOOST_AUTO_TEST_CASE(size_grows_with_each_insertion)
{
	const auto n = std::size_t{100};
	auto factory = minijava::ast_factory{};
	auto nodes = std::vector<std::unique_ptr<dummy_ast_node>>{};
	std::generate_n(
		std::back_inserter(nodes), n,
		[&factory](){ return factory.make<dummy_ast_node>()(); }
	);
	auto atmap = minijava::ast_attributes<int>{};
	for (auto i = std::size_t{0}; i < n; ++i) {
		atmap.insert({nodes.at(i).get(), 0});
		BOOST_REQUIRE_EQUAL(i + 1, atmap.size());
	}
}


namespace /* anonymous */
{

	void fill_containers(std::vector<std::unique_ptr<dummy_ast_node>>& nodes,
	                     std::vector<int>& values,
	                     minijava::ast_attributes<int>& atmap)
	{
		auto engine = std::default_random_engine{};
		auto dist = std::uniform_int_distribution<int>{};
		const auto n = std::size_t{1000};
		auto factory = minijava::ast_factory{};
		std::generate_n(
			std::back_inserter(nodes), n,
			[&factory](){ return factory.make<dummy_ast_node>()(); }
		);
		std::generate_n(
			std::back_inserter(values), n,
			[&engine, &dist](){ return dist(engine); }
		);
		for (auto i = std::size_t{0}; i < n; ++i) {
			atmap.insert({nodes.at(i).get(), values.at(i)});
		}
	}

	template <typename IterT>
	int check_containers(const std::vector<std::unique_ptr<dummy_ast_node>>& nodes,
						  const std::vector<int>& values,
						  const IterT it)
	{
		const auto pos = std::find_if(
			std::begin(nodes), std::end(nodes),
			[it](auto&& n){ return n.get() == it->first; }
		);
		if (pos == nodes.cend()) {
			return 1;
		}
		const auto offset = static_cast<std::size_t>(pos - nodes.cbegin());
		if (values.at(offset) != it->second) {
			return 2;
		}
		return 0;
	}

}


BOOST_AUTO_TEST_CASE(iterators_begin_end)
{
	auto nodes = std::vector<std::unique_ptr<dummy_ast_node>>{};
	auto values = std::vector<int>{};
	auto atmap = minijava::ast_attributes<int>{};
	fill_containers(nodes, values, atmap);
	for (auto it = atmap.begin(); it != atmap.end(); ++it) {
		BOOST_REQUIRE_EQUAL(0, check_containers(nodes, values, it));
	}
}


BOOST_AUTO_TEST_CASE(iterators_cbegin_cend)
{
	auto nodes = std::vector<std::unique_ptr<dummy_ast_node>>{};
	auto values = std::vector<int>{};
	auto atmap = minijava::ast_attributes<int>{};
	fill_containers(nodes, values, atmap);
	for (auto it = atmap.cbegin(); it != atmap.cend(); ++it) {
		BOOST_REQUIRE_EQUAL(0, check_containers(nodes, values, it));
	}
}


BOOST_AUTO_TEST_CASE(iterators_begin_end_const)
{
	auto nodes = std::vector<std::unique_ptr<dummy_ast_node>>{};
	auto values = std::vector<int>{};
	auto atmap = minijava::ast_attributes<int>{};
	fill_containers(nodes, values, atmap);
	for (auto it = testaux::as_const(atmap).begin(); it != testaux::as_const(atmap).end(); ++it) {
		BOOST_REQUIRE_EQUAL(0, check_containers(nodes, values, it));
	}
}


BOOST_AUTO_TEST_CASE(find_mutable)
{
	auto nodes = std::vector<std::unique_ptr<dummy_ast_node>>{};
	auto values = std::vector<int>{};
	auto atmap = minijava::ast_attributes<int>{};
	nodes.push_back(minijava::ast_builder<dummy_ast_node>{1 + nodes.size()}());
	fill_containers(nodes, values, atmap);
	for (const auto& n : nodes) {
		const auto stlpos = std::find_if(
			std::begin(atmap), std::end(atmap),
			[needle = n.get()](auto&& kv){ return kv.first == needle; }
		);
		const auto mappos = atmap.find(n.get());
		BOOST_REQUIRE(stlpos == mappos);
	}
}


BOOST_AUTO_TEST_CASE(find_const)
{
	auto nodes = std::vector<std::unique_ptr<dummy_ast_node>>{};
	auto values = std::vector<int>{};
	auto atmap = minijava::ast_attributes<int>{};
	nodes.push_back(minijava::ast_builder<dummy_ast_node>{1 + nodes.size()}());
	fill_containers(nodes, values, atmap);
	for (const auto& n : nodes) {
		const auto stlpos = std::find_if(
			std::begin(atmap), std::end(atmap),
			[needle = n.get()](auto&& kv){ return kv.first == needle; }
		);
		const auto mappos = testaux::as_const(atmap).find(n.get());
		BOOST_REQUIRE(stlpos == mappos);
	}
}


BOOST_AUTO_TEST_CASE(count)
{
	auto nodes = std::vector<std::unique_ptr<dummy_ast_node>>{};
	auto values = std::vector<int>{};
	auto atmap = minijava::ast_attributes<int>{};
	nodes.push_back(minijava::ast_builder<dummy_ast_node>{1 + nodes.size()}());
	fill_containers(nodes, values, atmap);
	for (const auto& n : nodes) {
		const auto stlcnt = std::count_if(
			std::begin(atmap), std::end(atmap),
			[needle = n.get()](auto&& kv){ return kv.first == needle; }
		);
		const auto mapcnt = testaux::as_const(atmap).count(n.get());
		BOOST_REQUIRE_EQUAL(stlcnt, mapcnt);
	}
}


BOOST_AUTO_TEST_CASE(subscript_operator_returns_existing_element)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<int>{};
	atmap.insert({nodeptr.get(), 42});
	BOOST_REQUIRE_EQUAL(42, atmap[*nodeptr]);
}


BOOST_AUTO_TEST_CASE(subscript_operator_value_initialzes_non_existing_element)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<int>{};
	BOOST_REQUIRE_EQUAL(0, atmap[*nodeptr]);
}


BOOST_AUTO_TEST_CASE(at_returns_existing_element)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<int>{};
	atmap.insert({nodeptr.get(), 42});
	BOOST_REQUIRE_EQUAL(42, atmap.at(*nodeptr));
	BOOST_REQUIRE_EQUAL(42, testaux::as_const(atmap).at(*nodeptr));
}


BOOST_AUTO_TEST_CASE(at_throws_for_non_existing_element)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<int>{};
	BOOST_REQUIRE_THROW(atmap.at(*nodeptr), std::out_of_range);
	BOOST_REQUIRE_THROW(testaux::as_const(atmap).at(*nodeptr), std::out_of_range);
}


BOOST_AUTO_TEST_CASE(insert_or_assign_inserts_new_element)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<not_default_constructible>{};
	const auto value = not_default_constructible{42};
	atmap.insert_or_assign(*nodeptr, value);
	BOOST_REQUIRE_EQUAL(1, atmap.size());
	BOOST_REQUIRE_EQUAL(value.tag, atmap.at(*nodeptr).tag);
}


BOOST_AUTO_TEST_CASE(insert_or_assign_overwrites_existing_element)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<not_default_constructible>{};
	const auto value = not_default_constructible{42};
	atmap.insert({nodeptr.get(), not_default_constructible{0}});
	atmap.insert_or_assign(*nodeptr, value);
	BOOST_REQUIRE_EQUAL(1, atmap.size());
	BOOST_REQUIRE_EQUAL(value.tag, atmap.at(*nodeptr).tag);
}


BOOST_AUTO_TEST_CASE(put_inserts_new_element)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<not_default_constructible>{};
	const auto value = not_default_constructible{42};
	atmap.put(*nodeptr, value);
	BOOST_REQUIRE_EQUAL(1, atmap.size());
	BOOST_REQUIRE_EQUAL(value.tag, atmap.at(*nodeptr).tag);
}


BOOST_AUTO_TEST_CASE(put_overwrites_existing_element)
{
	auto nodeptr = minijava::ast_factory{}.make<dummy_ast_node>()();
	auto atmap = minijava::ast_attributes<not_default_constructible>{};
	const auto value = not_default_constructible{42};
	atmap.insert({nodeptr.get(), value});
	BOOST_REQUIRE_THROW(
		atmap.put(*nodeptr, not_default_constructible{10}),
		std::out_of_range
	);
	BOOST_REQUIRE_EQUAL(1, atmap.size());
	BOOST_REQUIRE_EQUAL(value.tag, atmap.at(*nodeptr).tag);
}


BOOST_AUTO_TEST_CASE(filter_is_a_thing)
{
	auto engine = std::default_random_engine{};
	auto dist = std::uniform_int_distribution<int>{};
	auto flag = 0;
	const auto atmap = minijava::ast_attributes<int, mock_filter>{mock_filter{&flag}};
	for (auto i = 0; i < 10; ++i) {
		BOOST_REQUIRE_EQUAL(flag, atmap.get_filter().dynamic_check(0));
		flag = dist(engine);
	}
}


BOOST_AUTO_TEST_CASE(allocator_is_a_thing)
{
	using alloc_type = std::allocator<std::pair<const ast::node*const, int>>;
	const auto alloc = alloc_type{};
	const auto atmap = minijava::ast_attributes<int, minijava::ast_node_filter<ast::node>, alloc_type>{alloc};
	BOOST_REQUIRE(alloc == atmap.get_allocator());
	BOOST_REQUIRE_NO_THROW(atmap.get_allocator().deallocate(atmap.get_allocator().allocate(0), 0));
}


BOOST_AUTO_TEST_CASE(hash_function_is_a_thing)
{
	const auto atmap = minijava::ast_attributes<int>{};
	auto nodeptr = minijava::ast_builder<dummy_ast_node>{42}();
	BOOST_REQUIRE_EQUAL(42, atmap.hash_function()(nodeptr.get()));
}


BOOST_AUTO_TEST_CASE(key_eq_is_a_thing)
{
	const auto atmap = minijava::ast_attributes<int>{};
	auto nodeptr1st = minijava::ast_builder<dummy_ast_node>{42}();
	auto nodeptr2nd = minijava::ast_builder<dummy_ast_node>{42}();
	BOOST_REQUIRE(    atmap.key_eq()(nodeptr1st.get(), nodeptr1st.get()));
	BOOST_REQUIRE(not atmap.key_eq()(nodeptr1st.get(), nodeptr2nd.get()));
	BOOST_REQUIRE(not atmap.key_eq()(nodeptr2nd.get(), nodeptr1st.get()));
	BOOST_REQUIRE(    atmap.key_eq()(nodeptr2nd.get(), nodeptr2nd.get()));
}
