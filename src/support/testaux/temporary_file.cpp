#include "temporary_file.hpp"

#include <cerrno>
#include <fstream>
#include <iostream>
#include <system_error>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>


namespace testaux
{

	temporary_file::temporary_file(const std::string& text)
	{
		namespace fs = boost::filesystem;
		const auto path = fs::unique_path(fs::temp_directory_path() / "%%%%%%%%");
		_filename = path.string();
		std::ofstream stream{_filename, std::ios_base::binary | std::ios_base::out};
		stream.exceptions(std::iostream::failbit);
		stream << text << std::flush;
	}

	temporary_file::~temporary_file()
	{
		namespace fs = boost::filesystem;
		const auto path = fs::path{_filename};
		auto ec = boost::system::error_code{};
		if (fs::remove(path, ec) && ec) {
			// We cannot throw an exception from a destructor.
			std::cerr << "Cannot remove temporary file: " << _filename
			          << ": " << ec.message() << "\n";
		}
	}

}  // namespace testaux
