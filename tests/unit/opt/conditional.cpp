#include "opt/conditional.hpp"
#include "opt/opt.hpp"


#define BOOST_TEST_MODULE  opt_conditional
#include <boost/test/unit_test.hpp>

#include "irg/irg.hpp"
#include "semantic/semantic.hpp"

#include "testaux/ast_test_factory.hpp"
#include "testaux/temporary_file.hpp"


BOOST_AUTO_TEST_CASE(demo)
{
	testaux::temporary_directory dumpdir_before{};
	testaux::temporary_directory dumpdir_after{};
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	const auto seminfo = minijava::check_program(*ast, tf.pool, tf.factory);
	auto firm = minijava::initialize_firm();
	auto irg = minijava::create_firm_ir(*firm, *ast, seminfo, "test");
	minijava::dump_firm_ir(irg, dumpdir_before.filename());
	minijava::optimize(irg);
	minijava::dump_firm_ir(irg, dumpdir_after.filename());
}
