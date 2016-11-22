#include "meta/meta.hpp"

#include <tuple>
#include <type_traits>

#define BOOST_TEST_MODULE  meta_meta
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(conjunction)
{
	static_assert(
		minijava::meta::conjunction<>{},
		"The value of the empty product ought to be 1"
	);
	static_assert(
		not minijava::meta::conjunction<std::false_type>{},
		"The value of the singleton product 0 ought to be 0"
	);
	static_assert(
		minijava::meta::conjunction<std::true_type>{},
		"The value of the singleton product 1 ought to be 1"
	);
	static_assert(
		not minijava::meta::conjunction<std::false_type, std::true_type>{},
		"The value of the product 0 * 1 ought to be 0"
	);
	static_assert(
		not minijava::meta::conjunction<std::true_type, std::false_type>{},
		"The value of the product 1 * 0 ought to be 0"
	);
	static_assert(
		minijava::meta::conjunction<std::true_type, std::true_type>{},
		"The value of the product 1 * 1 ought to be 1"
	);
	static_assert(minijava::meta::conjunction<
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type
		>{},
		"The value of the product 1 * ... * 1 ought to be 1"
	);
	static_assert(not minijava::meta::conjunction<
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::true_type, std::true_type,
			std::true_type, std::true_type, std::false_type, std::true_type
		>{},
		"The value of the product 1 * ... * 0 * 1 ought to be 0"
	);
}


BOOST_AUTO_TEST_CASE(disjunction)
{
	static_assert(
		not minijava::meta::disjunction<>{},
		"The value of the empty sum ought to be 0"
	);
	static_assert(
		not minijava::meta::disjunction<std::false_type>{},
		"The value of the singleton sum 0 ought to be 0"
	);
	static_assert(
		minijava::meta::disjunction<std::true_type>{},
		"The value of the singleton sum 1 ought to be 1"
	);
	static_assert(
		minijava::meta::disjunction<std::false_type, std::true_type>{},
		"The value of the sum 0 + 1 ought to be 1"
	);
	static_assert(
		minijava::meta::disjunction<std::true_type, std::false_type>{},
		"The value of the sum 1 + 0 ought to be 1"
	);
	static_assert(
		minijava::meta::disjunction<std::true_type, std::true_type>{},
		"The value of the sum 1 + 1 ought to be 1"
	);
	static_assert(not minijava::meta::disjunction<
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type
		>{},
		"The value of the sum 0 + ... + 0 ought to be 0"
	);
	static_assert(minijava::meta::disjunction<
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::false_type, std::false_type,
			std::false_type, std::false_type, std::true_type, std::false_type
		>{},
		"The value of the sum 0 + ... + 1 + 0 ought to be 1"
	);
}


BOOST_AUTO_TEST_CASE(negate)
{
	static_assert(
		minijava::meta::negate<std::false_type>{},
		"The negated value of 0 ought to be 1"
	);
	static_assert(
		not minijava::meta::negate<std::true_type>{},
		"The negated value of 1 ought to be 0"
	);
}


BOOST_AUTO_TEST_CASE(lazy_valuation)
{
	struct XXX {};
	static_assert(
		not minijava::meta::conjunction<std::false_type, XXX, std::true_type>{},
		"Conjunction not evaluated lazily?"
	);
	static_assert(
		minijava::meta::disjunction<std::true_type, XXX, std::false_type>{},
		"Conjunction not evaluated lazily?"
	);
}


BOOST_AUTO_TEST_CASE(all_with_meta_types_t)
{
	using inputs = minijava::meta::types_t<int, float, double, long>;
	static_assert(minijava::meta::all<std::is_arithmetic>(inputs{}), "");
	static_assert(not minijava::meta::all<std::is_integral>(inputs{}), "");
}


BOOST_AUTO_TEST_CASE(all_with_std_tuple)
{
	constexpr auto inputs = std::tuple<int, float, double, long>{};
	static_assert(minijava::meta::all<std::is_arithmetic>(inputs), "");
	static_assert(not minijava::meta::all<std::is_integral>(inputs), "");
}


BOOST_AUTO_TEST_CASE(any_with_meta_types_t)
{
	using inputs = minijava::meta::types_t<int, float, double, long, void>;
	static_assert(minijava::meta::any<std::is_void>(inputs{}), "");
	static_assert(not minijava::meta::any<std::is_class>(inputs{}), "");
}


BOOST_AUTO_TEST_CASE(any_with_std_tuple)
{
	constexpr auto inputs = std::tuple<int, float, double, long>{};
	static_assert(minijava::meta::any<std::is_floating_point>(inputs), "");
	static_assert(not minijava::meta::any<std::is_union>(inputs), "");
}


BOOST_AUTO_TEST_CASE(none_with_meta_types_t)
{
	using inputs = minijava::meta::types_t<int, float, double, long>;
	static_assert(not minijava::meta::none<std::is_floating_point>(inputs{}), "");
	static_assert(minijava::meta::none<std::is_void>(inputs{}), "");
}


BOOST_AUTO_TEST_CASE(none_with_std_tuple)
{
	constexpr auto inputs = std::tuple<int, float, double, long>{};
	static_assert(not minijava::meta::none<std::is_arithmetic>(inputs), "");
	static_assert(minijava::meta::none<std::is_union>(inputs), "");
}
