#include "asm/asm.hpp"

#define BOOST_TEST_MODULE  asm_asm
#include <boost/test/unit_test.hpp>

#include "exceptions.hpp"
#include "io/file_output.hpp"
#include "irg/irg.hpp"
#include "opt/opt.hpp"
#include "semantic/semantic.hpp"

#include "testaux/ast_test_factory.hpp"
#include "testaux/temporary_file.hpp"


BOOST_AUTO_TEST_CASE(demo)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	const auto seminfo = minijava::check_program(*ast, tf.pool, tf.factory);
	auto firm = minijava::initialize_firm();
	auto irg = minijava::create_firm_ir(*firm, *ast, seminfo, "test");
	minijava::optimize(irg);
	testaux::temporary_file tempfile{};
	auto asmfile = minijava::file_output{tempfile.filename()};
	minijava::assemble(irg, asmfile);
}
