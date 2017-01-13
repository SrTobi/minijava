#include "temporary_file.hpp"

#include <algorithm>
#include <cerrno>
#include <fstream>
#include <iostream>
#include <iterator>
#include <system_error>

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>


namespace testaux
{

	temporary_file::temporary_file(const std::string& text, const std::string& suffix)
	{
		namespace fs = boost::filesystem;
		const auto pattern = "%%%%%%%%" + suffix;
		const auto path = fs::unique_path(fs::temp_directory_path() / pattern);
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

	temporary_directory::temporary_directory()
	{
		namespace fs = boost::filesystem;
		const auto path = fs::temp_directory_path() / fs::unique_path();
		_filename = path.string();
		if (!fs::create_directory(path)) {
			const auto ec = std::error_code{EEXIST, std::generic_category()};
			throw std::system_error{ec};
		}
	}

	temporary_directory::~temporary_directory()
	{
		namespace fs = boost::filesystem;
		const auto path = fs::path{_filename};
		auto ec = boost::system::error_code{};
		if (!fs::remove_all(path, ec) || ec) {
			// We cannot throw an exception from a destructor.
			std::cerr << "Cannot remove temporary directory: " << _filename
			          << ": " << ec.message() << "\n";
		}
	}

	std::string temporary_directory::filename(const std::string& local) const
	{
		namespace fs = boost::filesystem;
		const auto path = fs::path{_filename} / local;
		return path.string();
	}


	bool file_has_content(const std::string& filename, const std::string& expected)
	{
		std::ifstream istr{filename, std::ios_base::binary | std::ios_base::in};
		if (!istr) {
			throw std::ios_base::failure{"Cannot open file: " + filename};
		}
		istr.exceptions(std::ios_base::badbit);
		const auto first_1 = std::istreambuf_iterator<char>{istr};
		const auto last_1 = std::istreambuf_iterator<char>{};
		const auto first_2 = std::begin(expected);
		const auto last_2 = std::end(expected);
		return std::equal(first_1, last_1, first_2, last_2);
	}

	std::unique_ptr<std::FILE, decltype(&std::fclose)>
	open_file(const std::string& filename, const std::string& mode)
	{
		auto fp = std::unique_ptr<std::FILE, decltype(&std::fclose)>{
			std::fopen(filename.c_str(), mode.c_str()),
			&std::fclose
		};
		if (!fp) {
			const auto ec = std::error_code{errno, std::system_category()};
			throw std::system_error{ec, "Cannot open file: " + filename};
		}
		return fp;
	}

}  // namespace testaux
