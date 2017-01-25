#include "asm/text.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>

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
	auto virtasm = minijava::backend::virtual_assembly{};
	auto realasm = minijava::backend::real_assembly{};
	virtasm.emplace_back();
	virtasm.back().label = "foo";
	virtasm.emplace_back(opc::op_ret);
	BOOST_REQUIRE_THROW(
		minijava::backend::allocate_registers(virtasm, realasm),
		minijava::not_implemented_error
	);
}


BOOST_AUTO_TEST_CASE(write_text_for_nothing)
{
	auto assembly = minijava::backend::real_assembly{};
	testaux::temporary_file tempfile{};
	auto asmfile = minijava::file_output{tempfile.filename()};
	minijava::backend::write_text(assembly, asmfile);
	BOOST_CHECK(testaux::file_has_content(tempfile.filename(), ""));
}


BOOST_AUTO_TEST_CASE(write_text_for_empty_function)
{
	using opc = minijava::backend::opcode;
	using bw = minijava::backend::bit_width;
	using rr = minijava::backend::real_register;
	auto assembly = minijava::backend::real_assembly{};
	assembly.emplace_back();
	assembly.back().label = "func";
	assembly.emplace_back(opc::op_push, bw::lxiv, rr::bp);
	assembly.emplace_back(opc::op_mov, bw::lxiv, rr::sp, rr::bp);
	assembly.emplace_back(opc::op_sub, bw::lxiv, 0, rr::sp);
	assembly.emplace_back(opc::op_mov, bw::lxiv, rr::bp, rr::sp);
	assembly.emplace_back(opc::op_pop, bw::lxiv, rr::bp);
	assembly.emplace_back(opc::op_ret);
	const auto expected = std::string{}
		+ "func:\n"
		+ "\tpushq %rbp\n"
		+ "\tmovq %rsp, %rbp\n"
		+ "\tsubq $0, %rsp\n"
		+ "\tmovq %rbp, %rsp\n"
		+ "\tpopq %rbp\n"
		+ "\tret\n";
	testaux::temporary_file tempfile{};
	auto asmfile = minijava::file_output{tempfile.filename()};
	minijava::backend::write_text(assembly, asmfile);
	asmfile.close();
	BOOST_REQUIRE(testaux::file_has_content(tempfile.filename(), expected));
}

BOOST_AUTO_TEST_CASE(write_text_for_arbitrary_crap)
{
	using opc = minijava::backend::opcode;
	using bw = minijava::backend::bit_width;
	using rr = minijava::backend::real_register;
	auto assembly = minijava::backend::real_assembly{};
	assembly.emplace_back(opc::op_push, bw::xxxii, "button");
	assembly.emplace_back(opc::op_push, bw::xxxii, 42);
	const minijava::backend::address<rr> addresses[] = {
		{1234},
		{boost::none, rr::a},
		{boost::none, boost::none, rr::b},
		{boost::none, boost::none, rr::b, 1},
		{-4, boost::none, rr::b},
		{boost::none, rr::a, rr::b},
		{boost::none, rr::a, rr::b, 2},
		{-8, boost::none, rr::b, 4},
		{0, rr::a, rr::b, 4},
	};
	std::transform(
		std::begin(addresses), std::end(addresses),
		std::back_inserter(assembly),
		[](auto addr){
			return minijava::backend::instruction<rr>{opc::op_push, bw::xxxii, std::move(addr)};
		}
	);
	const auto expected = std::string{}
		+ "\tpushl button\n"
		+ "\tpushl $42\n"
		+ "\tpushl 1234\n"
		+ "\tpushl (%eax)\n"
		+ "\tpushl (, %ebx)\n"
		+ "\tpushl (, %ebx, 1)\n"
		+ "\tpushl -4(, %ebx)\n"
		+ "\tpushl (%eax, %ebx)\n"
		+ "\tpushl (%eax, %ebx, 2)\n"
		+ "\tpushl -8(, %ebx, 4)\n"
		+ "\tpushl 0(%eax, %ebx, 4)\n";
	testaux::temporary_file tempfile{};
	auto asmfile = minijava::file_output{tempfile.filename()};
	minijava::backend::write_text(assembly, asmfile);
	asmfile.close();
	BOOST_REQUIRE(testaux::file_has_content(tempfile.filename(), expected));
}
