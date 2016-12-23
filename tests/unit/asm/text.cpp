#include "asm/text.hpp"

#define BOOST_TEST_MODULE  asm_text
#include <boost/test/unit_test.hpp>

#include "exceptions.hpp"
#include "firm.hpp"
#include "irg/irg.hpp"

#include "testaux/temporary_file.hpp"


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
	auto virtasm = minijava::backend::virtual_assembly{};
	BOOST_REQUIRE_THROW(
		minijava::backend::assemble_function(irg, virtasm),
		minijava::not_implemented_error
	);
}


BOOST_AUTO_TEST_CASE(allocate_registers_for_empty_function)
{
	using opc = minijava::backend::opcode;
	using bw = minijava::backend::bit_width;
	using vreg = minijava::backend::virtual_register;
	auto virtasm = minijava::backend::virtual_assembly{};
	auto realasm = minijava::backend::real_assembly{};
	virtasm.emplace_back();
	virtasm.back().label = "foo";
	virtasm.emplace_back(opc::op_push, bw::lxiv, vreg::base_pointer);
	virtasm.emplace_back(opc::op_mov, bw::lxiv, vreg::stack_pointer, vreg::base_pointer);
	virtasm.emplace_back(opc::op_sub, bw::lxiv, 0, vreg::stack_pointer);
	virtasm.emplace_back(opc::op_mov, bw::lxiv, vreg::base_pointer, vreg::stack_pointer);
	virtasm.emplace_back(opc::op_pop, bw::lxiv, vreg::base_pointer);
	virtasm.emplace_back(opc::op_ret);
	BOOST_REQUIRE_THROW(
		minijava::backend::allocate_registers(virtasm, realasm),
		minijava::not_implemented_error
	);
}


BOOST_AUTO_TEST_CASE(write_text_for_empty_function)
{
	using opc = minijava::backend::opcode;
	using bw = minijava::backend::bit_width;
	using minijava::backend::real_register;
	auto assembly = minijava::backend::real_assembly{};
	assembly.emplace_back();
	assembly.back().label = "foo";
	assembly.emplace_back(opc::op_push, bw::lxiv, real_register::bp);
	assembly.emplace_back(opc::op_mov, bw::lxiv, real_register::sp, real_register::bp);
	assembly.emplace_back(opc::op_sub, bw::lxiv, 0, real_register::sp);
	assembly.emplace_back(opc::op_mov, bw::lxiv, real_register::bp, real_register::sp);
	assembly.emplace_back(opc::op_pop, bw::lxiv, real_register::bp);
	assembly.emplace_back(opc::op_ret);
	testaux::temporary_file tempfile{};
	auto asmfile = minijava::file_output{tempfile.filename()};
	BOOST_REQUIRE_THROW(
		minijava::backend::write_text(assembly, asmfile),
		minijava::not_implemented_error
	);
}
