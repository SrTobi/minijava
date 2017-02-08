#include "asm/data.hpp"

#define BOOST_TEST_MODULE  asm_data
#include <boost/test/unit_test.hpp>

#include "exceptions.hpp"
#include "firm.hpp"
#include "io/file_output.hpp"
#include "irg/irg.hpp"

#include "testaux/temporary_file.hpp"


BOOST_AUTO_TEST_CASE(demo)
{
	auto firm = minijava::initialize_firm();
	firm::lower_highlevel();
	testaux::temporary_file tempfile{};
	auto asmfile = minijava::file_output{tempfile.filename()};
	minijava::backend::write_data_segment(firm::get_glob_type(), asmfile);
}
