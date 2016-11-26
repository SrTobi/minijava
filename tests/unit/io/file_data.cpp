#include "io/file_data.hpp"

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <iterator>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

#define BOOST_TEST_MODULE  io_file_data.hpp
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "testaux/random.hpp"
#include "testaux/temporary_file.hpp"


BOOST_AUTO_TEST_CASE(type_properties)
{
	static_assert(std::is_nothrow_default_constructible<minijava::file_data>{}, "");
	static_assert(std::is_nothrow_move_constructible<minijava::file_data>{}, "");
	static_assert(std::is_nothrow_move_assignable<minijava::file_data>{}, "");
	static_assert(!std::is_copy_constructible<minijava::file_data>{}, "");
	static_assert(!std::is_copy_assignable<minijava::file_data>{}, "");
	static_assert(std::is_final<minijava::file_data>{}, "");
	static_assert(std::is_same<
			std::random_access_iterator_tag,
			std::iterator_traits<minijava::file_data::iterator>::iterator_category
		>{}, "Random access iterator expected");
	static_assert(std::is_same<
			minijava::file_data::iterator,
			decltype(std::declval<minijava::file_data>().begin())
		>{}, "begin() returns other iterator than promised");
	static_assert(std::is_same<
			minijava::file_data::iterator,
			decltype(std::declval<minijava::file_data>().end())
		>{}, "end() returns other iterator than promised");
	static_assert(std::is_same<
			char,
			std::iterator_traits<minijava::file_data::iterator>::value_type
		>{}, "'char' iterator expected");
	static_assert(std::is_same<
			const char*,
			std::iterator_traits<minijava::file_data::iterator>::pointer
		>{}, "'const char*' pointer expected");
	static_assert(std::is_same<
			const char&,
			std::iterator_traits<minijava::file_data::iterator>::reference
		>{}, "'const char&' reference expected");
	static_assert(std::is_same<
				  std::ptrdiff_t,
			std::iterator_traits<minijava::file_data::iterator>::difference_type
		>{}, "'std::ptrdiff_t' difference expected");
}


namespace /* anonymous */
{

	bool is_in_empty_state(const minijava::file_data& fdat) noexcept
	{
		return !fdat
			&& (fdat.size() == 0)
			&& (fdat.begin() == fdat.end())
			&& (fdat.filename().empty());
	}

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(default_constructed_object_can_be_disposed_with_no_effect)
{
	minijava::file_data fdat{};
	BOOST_REQUIRE(is_in_empty_state(fdat));
	fdat.dispose();
	BOOST_REQUIRE(is_in_empty_state(fdat));
	fdat.dispose();
	BOOST_REQUIRE(is_in_empty_state(fdat));
}


BOOST_AUTO_TEST_CASE(default_constructed_object_begin_returns_end)
{
	const minijava::file_data fdat{};
	BOOST_REQUIRE(fdat.begin() == fdat.end());
}


BOOST_AUTO_TEST_CASE(default_constructed_object_data_returns_non_null_pointer)
{
	const minijava::file_data fdat{};
	BOOST_REQUIRE(nullptr != fdat.data());
}


BOOST_AUTO_TEST_CASE(default_constructed_object_size_returns_zero)
{
	const minijava::file_data fdat{};
	BOOST_REQUIRE_EQUAL(std::size_t{0}, fdat.size());
}


BOOST_AUTO_TEST_CASE(default_constructed_object_filename_returns_empty_string)
{
	using namespace std::string_literals;
	const minijava::file_data fdat{};
	BOOST_REQUIRE_EQUAL(""s, fdat.filename());
}


BOOST_AUTO_TEST_CASE(default_constructed_object_is_false)
{
	const minijava::file_data fdat{};
	BOOST_REQUIRE_EQUAL(false, static_cast<bool>(fdat));
	BOOST_REQUIRE(!fdat);
}


BOOST_AUTO_TEST_CASE(default_constructed_object_is_in_empty_state)
{
	const minijava::file_data fdat{};
	BOOST_REQUIRE(is_in_empty_state(fdat));
}


static const std::string test_data[] = {
	"",
	"a",
	"The quick brown fox jumps over the sleazy dog!",
	testaux::make_random_string(std::size_t{1}),
	testaux::make_random_string(std::size_t{10}),
	testaux::make_random_string(std::size_t{100}),
	testaux::make_random_string(std::size_t{1000}),
	testaux::make_random_string(std::size_t{10000}),
	testaux::make_random_string(std::size_t{100000}),
	testaux::make_random_string(std::size_t{1000000}),
};


BOOST_DATA_TEST_CASE(constructed_from_file_pointer_contains_file_data, test_data)
{
	const testaux::temporary_file temp{sample};
	const auto fp = testaux::open_file(temp.filename(), "rb");
	const minijava::file_data fdat{fp.get(), temp.filename()};
	BOOST_REQUIRE(bool(fdat));
	BOOST_REQUIRE_EQUAL(sample.size(), fdat.size());
	BOOST_REQUIRE(std::equal(
		std::begin(sample), std::end(sample),
		std::begin(fdat), std::end(fdat)
	));
	BOOST_REQUIRE(fdat.data() == std::addressof(*fdat.begin()));
	BOOST_REQUIRE_EQUAL(temp.filename(), fdat.filename());
}


BOOST_DATA_TEST_CASE(constructed_from_file_name_contains_file_data, test_data)
{
	using namespace std::string_literals;
	const testaux::temporary_file temp{sample};
	const minijava::file_data fdat{temp.filename()};
	BOOST_REQUIRE(bool(fdat));
	BOOST_REQUIRE_EQUAL(sample.size(), fdat.size());
	BOOST_REQUIRE(std::equal(
		std::begin(sample), std::end(sample),
		std::begin(fdat), std::end(fdat)
	));
	BOOST_REQUIRE(fdat.data() == std::addressof(*fdat.begin()));
	BOOST_REQUIRE_EQUAL(temp.filename(), fdat.filename());
}


BOOST_AUTO_TEST_CASE(empty_file_name_is_rejected)
{
	BOOST_REQUIRE_EXCEPTION(
		minijava::file_data{""},
		std::system_error,
		[](auto&& e){ return e.code().value() == EINVAL; }
	);
}


BOOST_AUTO_TEST_CASE(empty_informal_file_name_is_allowed)
{
	using namespace std::string_literals;
	const auto text = "sneaky squirrel"s;
	const testaux::temporary_file temp{text};
	auto fp = testaux::open_file(temp.filename(), "rb");
	const minijava::file_data fdat{fp.get(), ""};
	BOOST_REQUIRE_EQUAL(""s, fdat.filename());
	BOOST_REQUIRE_EQUAL(text.size(), fdat.size());
	BOOST_REQUIRE(std::equal(
		std::begin(text), std::end(text),
		std::begin(fdat), std::end(fdat)
	));
}


BOOST_AUTO_TEST_CASE(mmap_falls_back_to_stdio)
{
	// TODO: I don't know how to write a feasible test for this.  On POSIX, we
	//       could (with some effort) create a FIFO or something of the like
	//       but this is neither trivial nor portable.
}


BOOST_AUTO_TEST_CASE(move_construction)
{
	using namespace std::string_literals;
	const auto text = "was there before"s;
	const testaux::temporary_file temp{text};
	minijava::file_data fdat1{temp.filename()};
	const auto p = fdat1.data();
	minijava::file_data fdat2{std::move(fdat1)};
	BOOST_REQUIRE(is_in_empty_state(fdat1));
	BOOST_REQUIRE_EQUAL(temp.filename(), fdat2.filename());
	BOOST_REQUIRE_EQUAL(text.size(), fdat2.size());
	BOOST_REQUIRE_EQUAL(p, fdat2.data());
}


BOOST_AUTO_TEST_CASE(move_assignment)
{
	using namespace std::string_literals;
	const auto text = "was there before"s;
	const testaux::temporary_file temp1{"won't last long"};
	const testaux::temporary_file temp2{text};
	minijava::file_data fdat1{temp1.filename()};
	minijava::file_data fdat2{temp2.filename()};
	const auto p = fdat2.data();
	fdat1 = std::move(fdat2);
	BOOST_REQUIRE(is_in_empty_state(fdat2));
	BOOST_REQUIRE_EQUAL(temp2.filename(), fdat1.filename());
	BOOST_REQUIRE_EQUAL(text.size(), fdat1.size());
	BOOST_REQUIRE_EQUAL(p, fdat1.data());
}


BOOST_AUTO_TEST_CASE(swapping)
{
	using namespace std::string_literals;
	const auto text1 = "alpha beta gamma delta"s;
	const auto text2 = "1 2 3 4 5 6 7 8 9"s;
	const testaux::temporary_file temp1{text1};
	const testaux::temporary_file temp2{text2};
	minijava::file_data fdat1{temp1.filename()};
	minijava::file_data fdat2{temp2.filename()};
	const auto p1 = fdat1.data();
	const auto p2 = fdat2.data();
	swap(fdat1, fdat2);
	BOOST_REQUIRE_EQUAL(temp1.filename(), fdat2.filename());
	BOOST_REQUIRE_EQUAL(temp2.filename(), fdat1.filename());
	BOOST_REQUIRE_EQUAL(text1.size(), fdat2.size());
	BOOST_REQUIRE_EQUAL(text2.size(), fdat1.size());
	BOOST_REQUIRE_EQUAL(p1, fdat2.data());
	BOOST_REQUIRE_EQUAL(p2, fdat1.data());
}
