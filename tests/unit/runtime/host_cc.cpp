#include "runtime/host_cc.hpp"

#include <cstring>

#define BOOST_TEST_MODULE  runtime_host_cc
#include <boost/test/unit_test.hpp>

#include "io/file_data.hpp"
#include "io/file_output.hpp"
#include "system/subprocess.hpp"

#include "testaux/temporary_file.hpp"

#ifdef __unix__
#  define ELF_EH 1
#else
#  define ELF_EH 0
#endif

#ifdef _WIN32
#	define WINDOWS 1
#else
#	define WINDOWS 0
#endif


BOOST_AUTO_TEST_CASE(default_c_compiler_is_not_empty)
{
	BOOST_CHECK(!minijava::get_default_c_compiler().empty());
}


static const char simple_asm[] = R"asm(
.text
.globl minijava_main
minijava_main:
	pushq %rbp
	movq  %rsp, %rbp
	nop
	popq  %rbp
	ret
)asm";

// TBD: Maybe we want to remove this test again because it is too brittle for a
//      unit test.  For now, however, it already found a bug, so let's keep it.
BOOST_AUTO_TEST_CASE(link_runtime_can_assemble)
{
	using namespace std::string_literals;
	testaux::temporary_file outfile{};
	testaux::temporary_file asmfile{simple_asm, ".S"};
	minijava::link_runtime(
		minijava::get_default_c_compiler(),
		outfile.filename(),
		asmfile.filename()
	);
	auto artifact_name = outfile.filename();
	if (WINDOWS) {
		artifact_name += ".exe";
	}
	minijava::file_data executable{artifact_name};
	BOOST_REQUIRE(executable.size() > 4);
	if (ELF_EH) {
		const char magic[] = {0x7f, 'E', 'L', 'F'};
		BOOST_CHECK(std::memcmp(magic, executable.data(), 4) == 0);
	} else if (WINDOWS) {
		const char magic[] = {'M', 'Z'};
		BOOST_CHECK(std::memcmp(magic, executable.data(), 2) == 0);
	}
	BOOST_REQUIRE_NO_THROW(minijava::run_subprocess({outfile.filename()}));
}
