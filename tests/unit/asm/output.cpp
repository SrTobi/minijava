#include "asm/output.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>

#define BOOST_TEST_MODULE  asm_output
#include <boost/test/unit_test.hpp>

#include "global.hpp"
#include "testaux/temporary_file.hpp"


BOOST_AUTO_TEST_CASE(write_text_for_nothing)
{
	using namespace std::string_literals;
	auto assembly = minijava::backend::real_assembly{"foo"};
	testaux::temporary_file tempfile{};
	auto asmfile = minijava::file_output{tempfile.filename()};
	minijava::backend::write_text(assembly, asmfile);
	asmfile.close();
	const auto expected_posix = ""s
		+ "\t.type foo, @function\n"
		+ "foo:\n"
		+ "\t.size foo, .-foo\n";
	const auto expected_widows = ""s
		+ "\t.def foo; .scl 2; .type 32; .endef\n"
		+ "foo:\n";
	const auto expected = MINIJAVA_WINDOWS_ASSEMBLY
		? expected_widows
		: expected_posix;
	BOOST_CHECK(testaux::file_has_content(tempfile.filename(), expected));
}


BOOST_AUTO_TEST_CASE(empty_labels_are_not_printed)
{
	using namespace std::string_literals;
	auto assembly = minijava::backend::real_assembly{"func"};
	assembly.blocks.emplace_back(".L0");  // non-empty
	assembly.blocks.emplace_back("");     // empty
	testaux::temporary_file tempfile{};
	auto asmfile = minijava::file_output{tempfile.filename()};
	minijava::backend::write_text(assembly, asmfile);
	asmfile.close();
	const auto expected_posix = ""s
		+ "\t.type func, @function\n"
		+ "func:\n"
		+ ".L0:\n"
		+ "\t.size func, .-func\n";
	const auto expected_widows = ""s
		+ "\t.def func; .scl 2; .type 32; .endef\n"
		+ ".L0:\n"
		+ "func:\n";
	const auto expected = MINIJAVA_WINDOWS_ASSEMBLY
		? expected_widows
		: expected_posix;
	BOOST_CHECK(testaux::file_has_content(tempfile.filename(), expected));
}


BOOST_AUTO_TEST_CASE(write_text_for_empty_function)
{
	using namespace std::string_literals;
	using opc = minijava::backend::opcode;
	using bw = minijava::backend::bit_width;
	using rr = minijava::backend::real_register;
	auto assembly = minijava::backend::real_assembly{"func"};
	assembly.blocks.emplace_back(".L0");
	assembly.blocks.back().code.emplace_back(opc::op_push, bw::lxiv, rr::bp);
	assembly.blocks.back().code.emplace_back(opc::op_mov, bw::lxiv, rr::sp, rr::bp);
	assembly.blocks.back().code.emplace_back(opc::op_sub, bw::lxiv, 0, rr::sp);
	assembly.blocks.back().code.emplace_back(opc::op_mov, bw::lxiv, rr::bp, rr::sp);
	assembly.blocks.back().code.emplace_back(opc::op_pop, bw::lxiv, rr::bp);
	assembly.blocks.back().code.emplace_back(opc::op_ret);
	const auto expected = ""s
#if MINIJAVA_WINDOWS_ASSEMBLY
		+ "\t.def func; .scl 2; .type 32; .endef\n"
#else
		+ "\t.type func, @function\n"
#endif
		+ "func:\n"
		+ ".L0:\n"
		+ "\tpushq %rbp\n"
		+ "\tmovq %rsp, %rbp\n"
		+ "\tsubq $0, %rsp\n"
		+ "\tmovq %rbp, %rsp\n"
		+ "\tpopq %rbp\n"
		+ "\tret\n"
#if ! MINIJAVA_WINDOWS_ASSEMBLY
		+ "\t.size func, .-func\n"
#endif
		;
	testaux::temporary_file tempfile{};
	auto asmfile = minijava::file_output{tempfile.filename()};
	minijava::backend::write_text(assembly, asmfile);
	asmfile.close();
	BOOST_REQUIRE(testaux::file_has_content(tempfile.filename(), expected));
}


BOOST_AUTO_TEST_CASE(write_text_for_arbitrary_crap)
{
	using namespace std::string_literals;
	using opc = minijava::backend::opcode;
	using bw = minijava::backend::bit_width;
	using rr = minijava::backend::real_register;
	auto assembly = minijava::backend::real_assembly{"name"};
	assembly.blocks.emplace_back("");
	assembly.blocks.back().code.emplace_back(opc::op_push, bw::xxxii, "button");
	assembly.blocks.back().code.emplace_back(opc::op_call, bw{}, "me");
	assembly.blocks.back().code.emplace_back(opc::op_jmp, bw{}, "high");
	assembly.blocks.back().code.emplace_back(opc::op_push, bw::xxxii, 42);
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
		std::back_inserter(assembly.blocks.back().code),
		[](auto addr){
			return minijava::backend::instruction<rr>{opc::op_push, bw::xxxii, std::move(addr)};
		}
	);
	const auto expected = ""s
#if MINIJAVA_WINDOWS_ASSEMBLY
		+ "\t.def name; .scl 2; .type 32; .endef\n"
#else
		+ "\t.type name, @function\n"
#endif
		+ "name:\n"
		+ "\tpushl button\n"
		+ "\tcall me\n"
		+ "\tjmp high\n"
		+ "\tpushl $42\n"
		+ "\tpushl 1234\n"
		// Address computation always use 64 bit registers, regardless of the
		// instruction width.
		+ "\tpushl (%rax)\n"
		+ "\tpushl (, %rbx)\n"
		+ "\tpushl (, %rbx, 1)\n"
		+ "\tpushl -4(, %rbx)\n"
		+ "\tpushl (%rax, %rbx)\n"
		+ "\tpushl (%rax, %rbx, 2)\n"
		+ "\tpushl -8(, %rbx, 4)\n"
		+ "\tpushl 0(%rax, %rbx, 4)\n"
#if ! MINIJAVA_WINDOWS_ASSEMBLY
		+ "\t.size name, .-name\n"
#endif
		;
	testaux::temporary_file tempfile{};
	auto asmfile = minijava::file_output{tempfile.filename()};
	minijava::backend::write_text(assembly, asmfile);
	asmfile.close();
	BOOST_REQUIRE(testaux::file_has_content(tempfile.filename(), expected));
}
