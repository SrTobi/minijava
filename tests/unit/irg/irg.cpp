#include "irg/irg.hpp"

#define BOOST_TEST_MODULE  irg_irg
#include <boost/test/unit_test.hpp>

#include "semantic/semantic.hpp"
#include "testaux/ast_test_factory.hpp"


BOOST_AUTO_TEST_CASE(create_firm_ir_for_hello_world)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.make_hello_world();
	const auto seminfo = minijava::check_program(*ast, tf.pool, tf.factory);
	auto firm = minijava::initialize_firm();
	minijava::create_firm_ir(*firm, *ast, seminfo, "test");
}
