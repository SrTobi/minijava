#include "runtime/host_cc.hpp"

#include <cstdlib>

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

#if __APPLE__
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
		auto tmp_path = fs::temp_directory_path() / fs::unique_path();
		auto runtime_filename = tmp_path.string();
		auto runtime_file = file_output{runtime_filename};
		runtime_file.write(runtime_source());
		runtime_file.close();
		run_subprocess({
				compiler_executable,
		        "-o",
				output_filename,
				assembly_filename,
		        runtime_filename
		});
	}

}  // namespace minijava
