#include "asm/generator.hpp"

#define BOOST_TEST_MODULE  asm_generator
#include <boost/test/unit_test.hpp>

#include "exceptions.hpp"
#include "firm.hpp"
#include "irg/irg.hpp"


BOOST_AUTO_TEST_CASE(assemble_empty_function)
{
	auto firm = minijava::initialize_firm();
	const auto name = firm::new_id_from_str("foo");
	const auto method_type = firm::new_type_method(
		0, 0, 0, 0, firm::mtp_no_property
	);
	const auto method_entity = firm::new_entity(
		firm::get_glob_type(), name, method_type
	);
	firm::set_entity_ld_ident(method_entity, name);
	const auto irg = firm::new_ir_graph(method_entity, 0);
	firm::set_current_ir_graph(irg);
	firm::lower_highlevel();
	const auto virtasm = minijava::backend::assemble_function(irg);
	// TODO: Check something meaningful.
}
