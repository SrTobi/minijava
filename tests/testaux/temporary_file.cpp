#define _POSIX_C_SOURCE >= 200809L  // for mkstemp

#include "temporary_file.hpp"

#include <cerrno>
#include <fstream>
#include <iostream>
#include <system_error>

#include <unistd.h>


namespace testaux
{

	temporary_file::temporary_file(const std::string& text) {
		_filename = "tempfile-XXXXXX";
		const auto fd = mkstemp(&_filename.front());
		if ((fd < 0) || (close(fd) < 0)) {
			const auto ec = std::error_code{errno, std::system_category()};
			throw std::system_error{ec};
		}
		std::ofstream stream{_filename};
		stream.exceptions(std::iostream::failbit);
		stream << text << std::flush;
	}

	temporary_file::~temporary_file() {
		if (unlink(_filename.c_str()) < 0) {
			std::cerr << "Cannot unlink temporary file: " << _filename << "\n";
		}
	}

}  // namespace testaux
