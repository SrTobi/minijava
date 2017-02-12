#include "runtime/host_cc.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

#include <boost/filesystem.hpp>

#include "io/file_output.hpp"
#include "system/subprocess.hpp"

#include "runtime/runtime.hpp"

namespace minijava
{

	std::string get_default_c_compiler()
	{
		if (const char* compiler_binary = std::getenv("CC")) {
			return std::string(compiler_binary);
		}

#ifdef __APPLE__
		return "clang";
#else
		return "gcc";
#endif
	}

	void link_runtime(const std::string& compiler_executable,
	                  const std::string& output_filename,
	                  const std::string& assembly_filename)
	{
		namespace fs = boost::filesystem;
		using namespace std::string_literals;
		const auto pattern = fs::temp_directory_path() / "%%%%%%%%%%%%.c";
		auto tmp_path = fs::unique_path(pattern);
		auto runtime_filename = tmp_path.string();
		auto runtime_file = file_output{runtime_filename};
		runtime_file.write(runtime_source());
		runtime_file.close();
		try {
			run_subprocess({
					compiler_executable,
					"-g",
					/* On some systems, ld creates position-independent
					 * executables by default (for ASLR), which causes a linker
					 * error since our assembly is not position-independent.
					 * The easiest way to disable this behavior in a portable
					 * manner is to link everything statically. */
					"-static",
					"-m64",
					"-o",
					output_filename,
					assembly_filename,
					runtime_filename,
			});
		} catch (const std::exception& e) {
			throw std::runtime_error{
				"Cannot run host assembler and linker: "s + e.what()
			};
		}
	}

}  // namespace minijava
