#include "io/file_cleanup.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <utility>

#include <boost/filesystem.hpp>

#include "global.hpp"


namespace /*anonymous*/
{

	bool keep_temporary_files()
	{
		static const auto answer = [](){
			const auto envval = std::getenv(MINIJAVA_ENVVAR_KEEP_TEMPORARY_FILES);
			return (envval != nullptr) && (std::strlen(envval) > 0);
		}();
		return answer;
	}

}  // namespace /*anonymous*/


namespace minijava
{

	file_cleanup::file_cleanup(std::string filename)
		: _filename{std::move(filename)}
	{
	}

	file_cleanup::~file_cleanup()
	{
		namespace fs = boost::filesystem;
		if (!keep_temporary_files() && !_filename.empty()) {
			const auto path = fs::path{_filename};
			auto ec = boost::system::error_code{};
			if (fs::remove(path, ec) && ec) {
				std::cerr << "Cannot cleanup temporary file: " << _filename
						  << ": " << ec.message() << std::endl;
			}
		}
	}

}  // namespace minijava
