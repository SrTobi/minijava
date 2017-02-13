#include "io/file_cleanup.hpp"

#define BOOST_TEST_MODULE  io_file_cleanup
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "testaux/temporary_file.hpp"


BOOST_AUTO_TEST_CASE(non_existing_ok)
{
	const auto filename = testaux::temporary_file{}.filename();
	const minijava::file_cleanup guard{filename};
}


BOOST_AUTO_TEST_CASE(destructor_unlinks_file)
{
	namespace fs = boost::filesystem;
	const testaux::temporary_file tempfile{};
	{
		const minijava::file_cleanup guard{tempfile.filename()};
		BOOST_REQUIRE(fs::exists(fs::path{tempfile.filename()}));
	}
	BOOST_REQUIRE(!fs::exists(fs::path{tempfile.filename()}));
}
