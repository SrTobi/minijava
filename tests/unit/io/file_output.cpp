#include "io/file_output.hpp"

#include <cerrno>
#include <cstdio>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>

#define BOOST_TEST_MODULE  io_file_output.hpp
#include <boost/test/unit_test.hpp>

#include "testaux/random.hpp"
#include "testaux/temporary_file.hpp"
#include "testaux/testaux.hpp"


#ifdef __unix__
#  define POSIX_EH  true
#else
#  define POSIX_EH  false
#endif

#define REQUIRE_SYSTEM_ERROR_WITH_ERRNO(Expr, Errc)                           \
	BOOST_REQUIRE_EXCEPTION(                                                  \
		Expr,                                                                 \
		std::system_error,                                                    \
		[](auto&& e){ return e.code().value() == Errc; }                      \
	)


namespace /* anonymous */
{

	const void* voided(const void* p) noexcept
	{
		return p;
	}

	bool is_in_empty_state(minijava::file_output& out) noexcept
	{
		return !out
			&& out.filename().empty()
			&& (out.handle() == nullptr);
	}

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(type_properties)
{
	static_assert(std::is_nothrow_default_constructible<minijava::file_output>{}, "");
	static_assert(std::is_nothrow_move_constructible<minijava::file_output>{}, "");
	static_assert(std::is_nothrow_move_assignable<minijava::file_output>{}, "");
	static_assert(!std::is_copy_constructible<minijava::file_output>{}, "");
	static_assert(!std::is_copy_assignable<minijava::file_output>{}, "");
	static_assert(std::is_final<minijava::file_output>{}, "");
}


BOOST_AUTO_TEST_CASE(empty_state)
{
	using namespace std::string_literals;
	auto out = minijava::file_output{};
	BOOST_REQUIRE_EQUAL(false, bool(testaux::as_const(out)));
	BOOST_REQUIRE_EQUAL(""s, testaux::as_const(out).filename());
	BOOST_REQUIRE_EQUAL(voided(nullptr), voided(out.handle()));
	BOOST_REQUIRE(is_in_empty_state(out));  // sanity
}


BOOST_AUTO_TEST_CASE(empty_state_write_1st)
{
	auto out = minijava::file_output{};
	REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.write(""), EBADF);
}


BOOST_AUTO_TEST_CASE(empty_state_write_2nd)
{
	auto out = minijava::file_output{};
	REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.write("", 0), EBADF);
}


BOOST_AUTO_TEST_CASE(empty_state_print)
{
	auto out = minijava::file_output{};
	REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.print("%d", 42), EBADF);
}


BOOST_AUTO_TEST_CASE(empty_state_flush)
{
	auto out = minijava::file_output{};
	REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.flush(), EBADF);
}


BOOST_AUTO_TEST_CASE(empty_state_close)
{
	auto out = minijava::file_output{};
	REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.close(), EBADF);
}


BOOST_AUTO_TEST_CASE(empty_state_finalize)
{
	auto out = minijava::file_output{};
	out.finalize();
}


BOOST_AUTO_TEST_CASE(non_owning_state_no_filename)
{
	using namespace std::string_literals;
	testaux::temporary_file temp{};
	auto fh = testaux::open_file(temp.filename(), "wb");
	auto out = minijava::file_output{fh.get()};
	BOOST_REQUIRE_EQUAL(true, bool(testaux::as_const(out)));
	BOOST_REQUIRE_EQUAL(""s, testaux::as_const(out).filename());
	BOOST_REQUIRE_EQUAL(voided(fh.get()), voided(out.handle()));
}


BOOST_AUTO_TEST_CASE(non_owning_state_yes_filename)
{
	using namespace std::string_literals;
	testaux::temporary_file temp{};
	auto fh = testaux::open_file(temp.filename(), "wb");
	auto out = minijava::file_output{fh.get(), "whatever"};
	BOOST_REQUIRE_EQUAL(true, bool(testaux::as_const(out)));
	BOOST_REQUIRE_EQUAL("whatever"s, testaux::as_const(out).filename());
	BOOST_REQUIRE_EQUAL(voided(fh.get()), voided(out.handle()));
}


BOOST_AUTO_TEST_CASE(non_owning_state_write_1st_success)
{
	using namespace std::string_literals;
	const auto text = "a"s;  // < BUFSIZ
	testaux::temporary_file temp{};
	auto fh = testaux::open_file(temp.filename(), "wb");
	{
		auto out = minijava::file_output{fh.get()};
		out.write(text);
	}
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), text));
}


BOOST_AUTO_TEST_CASE(non_owning_state_write_2nd_success)
{
	using namespace std::string_literals;
	const auto text = "Please write these two\nlines of text to the file.\n"s;
	testaux::temporary_file temp{};
	auto fh = testaux::open_file(temp.filename(), "wb");
	{
		auto out = minijava::file_output{fh.get()};
		out.write(text.data(), text.size());
	}
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), text));
}


BOOST_AUTO_TEST_CASE(non_owning_state_write_failure)
{
	if (POSIX_EH) {
		const auto text = std::string(BUFSIZ + 100, 'a');
		auto fh = testaux::open_file("/dev/full", "wb");
		auto out = minijava::file_output{fh.get()};
		REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.write(text), ENOSPC);
	}
}


BOOST_AUTO_TEST_CASE(non_owning_state_print_success)
{
	using namespace std::string_literals;
	testaux::temporary_file temp{};
	auto fh = testaux::open_file(temp.filename(), "wb");
	{
		auto out = minijava::file_output{fh.get()};
		out.print("The %s is: %d", "answer", 42);
	}
	const auto expected = "The answer is: 42"s;
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), expected));
}


BOOST_AUTO_TEST_CASE(non_owning_state_print_failure)
{
	const auto aaa = std::string(std::size_t{BUFSIZ}, 'a');
	if (POSIX_EH) {
		auto fh = testaux::open_file("/dev/full", "wb");
		auto out = minijava::file_output{fh.get()};
		REQUIRE_SYSTEM_ERROR_WITH_ERRNO(
			out.print("%s%s", aaa.c_str(), aaa.c_str()),
			ENOSPC
		);
	}
}


BOOST_AUTO_TEST_CASE(non_owning_state_flush_success)
{
	testaux::temporary_file temp{};
	auto fh = testaux::open_file(temp.filename(), "wb");
	auto out = minijava::file_output{fh.get()};
	std::fputc('a', fh.get());
	out.flush();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "a"));
	std::fputc('b', fh.get());
	out.flush();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "ab"));
}


BOOST_AUTO_TEST_CASE(non_owning_state_flush_failure)
{
	if (POSIX_EH) {
		auto fh = testaux::open_file("/dev/full", "wb");
		auto out = minijava::file_output{fh.get()};
		out.write("a");  // BUFFERED, I HOPE HOPE HOPE
		REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.flush(), ENOSPC);
	}
}


BOOST_AUTO_TEST_CASE(non_owning_state_close_success)
{
	testaux::temporary_file temp{};
	auto fh = testaux::open_file(temp.filename(), "wb");
	auto out = minijava::file_output{fh.get()};
	std::fputc('a', fh.release());
	out.close();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "a"));
	BOOST_REQUIRE(is_in_empty_state(out));
}


BOOST_AUTO_TEST_CASE(non_owning_state_close_failure)
{
	if (POSIX_EH) {
		auto fh = testaux::open_file("/dev/full", "wb");
		auto out = minijava::file_output{fh.release()};
		out.write("a");  // BUFFERED, I HOPE HOPE HOPE
		REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.close(), ENOSPC);
	}
}


BOOST_AUTO_TEST_CASE(non_owning_state_finalize_success)
{
	testaux::temporary_file temp{};
	auto fh = testaux::open_file(temp.filename(), "wb");
	auto out = minijava::file_output{fh.get()};
	std::fputc('a', fh.get());
	out.finalize();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "a"));
	std::fputc('b', fh.get());
	out.finalize();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "ab"));
}


BOOST_AUTO_TEST_CASE(non_owning_state_finalize_failure)
{
	if (POSIX_EH) {
		auto fh = testaux::open_file("/dev/full", "wb");
		auto out = minijava::file_output{fh.get()};
		out.write("a");  // BUFFERED, I HOPE HOPE HOPE
		REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.finalize(), ENOSPC);
	}
}


BOOST_AUTO_TEST_CASE(non_owning_state_mess_with_handle)
{
	testaux::temporary_file temp{};
	{
		auto fh = testaux::open_file(temp.filename(), "wb");
		auto out = minijava::file_output{fh.get()};
		out.print("alpha");
		std::fprintf(fh.get(), "beta");
		out.print("gamma");
		std::fprintf(fh.get(), "delta");
	}
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "alphabetagammadelta"));
}


BOOST_AUTO_TEST_CASE(owning_state)
{
	using namespace std::string_literals;
	testaux::temporary_file temp{};
	auto out = minijava::file_output{temp.filename()};
	BOOST_REQUIRE_EQUAL(true, bool(testaux::as_const(out)));
	BOOST_REQUIRE_EQUAL(temp.filename(), testaux::as_const(out).filename());
	BOOST_REQUIRE_NE(voided(nullptr), voided(out.handle()));
}


BOOST_AUTO_TEST_CASE(owning_state_rejects_empty_filename)
{
	using namespace std::string_literals;
	REQUIRE_SYSTEM_ERROR_WITH_ERRNO(minijava::file_output{""}, EINVAL);
}


BOOST_AUTO_TEST_CASE(owning_state_write_1st_success)
{
	using namespace std::string_literals;
	const auto text = testaux::make_random_string(1000);
	testaux::temporary_file temp{};
	{
		auto out = minijava::file_output{temp.filename()};
		out.write(text);
	}
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), text));
}


BOOST_AUTO_TEST_CASE(owning_state_write_2nd_success)
{
	using namespace std::string_literals;
	const auto text = testaux::make_random_string(1000);
	testaux::temporary_file temp{};
	{
		auto out = minijava::file_output{temp.filename()};
		out.write(text.data(), text.size());
	}
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), text));
}


BOOST_AUTO_TEST_CASE(owning_state_write_failure)
{
	if (POSIX_EH) {
		const auto text = std::string(BUFSIZ + 100, 'a');
		auto out = minijava::file_output{"/dev/full"};
		REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.write(text), ENOSPC);
	}
}


BOOST_AUTO_TEST_CASE(owning_state_print_success)
{
	using namespace std::string_literals;
	testaux::temporary_file temp{};
	{
		auto out = minijava::file_output{temp.filename()};
		out.print("The %s is: %d", "answer", 42);
	}
	const auto expected = "The answer is: 42"s;
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), expected));
}


BOOST_AUTO_TEST_CASE(owning_state_print_failure)
{
	const auto aaa = std::string(std::size_t{BUFSIZ}, 'a');
	if (POSIX_EH) {
		auto out = minijava::file_output{"/dev/full"};
		REQUIRE_SYSTEM_ERROR_WITH_ERRNO(
			out.print("%s%s", aaa.c_str(), aaa.c_str()),
			ENOSPC
		);
	}
}


BOOST_AUTO_TEST_CASE(owning_state_flush_success)
{
	testaux::temporary_file temp{};
	auto out = minijava::file_output{temp.filename()};
	std::fputc('a', out.handle());
	out.flush();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "a"));
	std::fputc('b', out.handle());
	out.flush();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "ab"));
}


BOOST_AUTO_TEST_CASE(owning_state_flush_failure)
{
	if (POSIX_EH) {
		auto out = minijava::file_output{"/dev/full"};
		out.write("a");  // BUFFERED, I HOPE HOPE HOPE
		REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.flush(), ENOSPC);
	}
}


BOOST_AUTO_TEST_CASE(owning_state_close_success)
{
	testaux::temporary_file temp{};
	auto out = minijava::file_output{temp.filename()};
	std::fputc('a', out.handle());
	out.close();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "a"));
	BOOST_REQUIRE(is_in_empty_state(out));
}


BOOST_AUTO_TEST_CASE(owning_state_close_failure)
{
	if (POSIX_EH) {
		auto out = minijava::file_output{"/dev/full"};
		out.write("a");  // BUFFERED, I HOPE HOPE HOPE
		REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.close(), ENOSPC);
	}
}


BOOST_AUTO_TEST_CASE(owning_state_finalize_success)
{
	testaux::temporary_file temp{};
	auto out = minijava::file_output{temp.filename()};
	std::fputc('a', out.handle());
	out.finalize();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "a"));
	BOOST_REQUIRE(is_in_empty_state(out));
}


BOOST_AUTO_TEST_CASE(owning_state_finalize_failure)
{
	if (POSIX_EH) {
		auto out = minijava::file_output{"/dev/full"};
		out.write("a");  // BUFFERED, I HOPE HOPE HOPE
		REQUIRE_SYSTEM_ERROR_WITH_ERRNO(out.finalize(), ENOSPC);
	}
}


BOOST_AUTO_TEST_CASE(owning_state_mess_with_handle)
{
	testaux::temporary_file temp{};
	auto out = minijava::file_output{temp.filename()};
	auto fp = out.handle();
	out.print("alpha");
	std::fprintf(fp, "beta");
	out.print("gamma");
	std::fprintf(fp, "delta");
	out.close();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "alphabetagammadelta"));
}


BOOST_AUTO_TEST_CASE(move_construction)
{
	testaux::temporary_file temp{};
	minijava::file_output out1{temp.filename()};
	const auto fp = out1.handle();
	out1.write("before");
	minijava::file_output out2{std::move(out1)};
	BOOST_REQUIRE(is_in_empty_state(out1));
	out2.write("after");
	BOOST_REQUIRE_EQUAL(voided(fp), voided(out2.handle()));
	out2.close();
	BOOST_REQUIRE(testaux::file_has_content(temp.filename(), "beforeafter"));
}


BOOST_AUTO_TEST_CASE(move_assignment)
{
	testaux::temporary_file temp1{};
	testaux::temporary_file temp2{};
	minijava::file_output out1{temp1.filename()};
	minijava::file_output out2{temp2.filename()};
	const auto fp = out2.handle();
	out1.write("a");
	out2.write("b");
	out1 = std::move(out2);
	BOOST_REQUIRE(is_in_empty_state(out2));
	out1.write("c");
	BOOST_REQUIRE_EQUAL(voided(fp), voided(out1.handle()));
	out1.close();
	BOOST_REQUIRE(testaux::file_has_content(temp1.filename(), "a"));
	BOOST_REQUIRE(testaux::file_has_content(temp2.filename(), "bc"));
}


BOOST_AUTO_TEST_CASE(swapping)
{
	testaux::temporary_file temp1{};
	testaux::temporary_file temp2{};
	{
		auto fh2 = testaux::open_file(temp2.filename(), "wb");
		minijava::file_output out1{temp1.filename()};
		minijava::file_output out2{fh2.get(), temp2.filename()};
		const auto fp1 = out1.handle();
		out1.write("alpha");
		out2.write("beta");
		swap(out1, out2);
		out1.write("gamma");
		out2.write("delta");
		BOOST_REQUIRE_EQUAL(temp1.filename(), out2.filename());
		BOOST_REQUIRE_EQUAL(temp2.filename(), out1.filename());
		BOOST_REQUIRE_EQUAL(voided(fh2.get()), voided(out1.handle()));
		BOOST_REQUIRE_EQUAL(voided(fp1), voided(out2.handle()));
	}
	BOOST_REQUIRE(testaux::file_has_content(temp1.filename(), "alphadelta"));
	BOOST_REQUIRE(testaux::file_has_content(temp2.filename(), "betagamma"));
}
