#include "irg/globals.hpp"

#define BOOST_TEST_MODULE  irg_globals
#include <boost/test/unit_test.hpp>

#include "irg/irg.hpp"
#include "irg/type_builder.hpp"
#include "semantic/semantic.hpp"
#include "testaux/ast_test_factory.hpp"


BOOST_AUTO_TEST_CASE(demo)
{
	auto tf = testaux::ast_test_factory{};
	const auto ast = tf.as_program(tf.make_empty_main());
	const auto seminfo = minijava::check_program(*ast, tf.pool, tf.factory);
	auto firm = minijava::initialize_firm();
	auto types = minijava::irg::create_types(*ast, seminfo);
	BOOST_REQUIRE_NO_THROW(minijava::irg::create_globals(seminfo, types));
}
