// This is an input generator written in C++.
//
// Wrting a C++ program instead of a Python script has the benefit that it is
// faster.  While this is nice, it's probably not worth the additional effort.
// The real reason why this generator is written in C++ and not in Python is to
// show that the benchmark runner can handle generators in different
// directories.

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <vector>

#include <boost/program_options.hpp>


namespace /* anonymous */
{

	void dump_to_file_pointer(std::FILE *const fp, const std::size_t size)
	{
		using word_type = unsigned long;
		const auto word_size = sizeof(word_type);
		auto rndeng = std::default_random_engine{std::random_device{}()};
		auto rnddst = std::uniform_int_distribution<word_type>{};
		auto buffer = std::vector<word_type>(1024);
		std::generate(std::begin(buffer), std::end(buffer),
		             [&rndeng, &rnddst](){ return rnddst(rndeng); });
		auto count = std::size_t{};
		while (count < size) {
			const auto chunk = std::min(word_size * buffer.size(), size - count);
			const auto written = std::fwrite(buffer.data(), 1, chunk, fp);
			if (written == 0) {
				const auto ec = std::error_code{errno, std::system_category()};
				throw std::system_error{ec, "Cannot write data to file"};
			}
			count += written;
		}
	}

	void dump_to_file(const std::string& filename, const std::size_t size)
	{
		auto fh = std::unique_ptr<std::FILE, decltype(&std::fclose)>{
			(filename == "-") ? stdout : std::fopen(filename.c_str(), "wb"),
			&std::fclose
		};
		if (!fh) {
			const auto ec = std::error_code{errno, std::system_category()};
			throw std::system_error{ec, "Cannot open file for writing"};
		}
		dump_to_file_pointer(fh.get(), size);
		if (std::fclose(fh.release()) < 0) {
			const auto ec = std::error_code{errno, std::system_category()};
			throw std::system_error{ec, "Cannot close file"};
		}
	}

	void real_main(const std::vector<const char*>& args)
	{
		namespace po = boost::program_options;
		auto filename = std::string{"-"};
		auto size = std::size_t{};
		auto options = po::options_description{"Options"};
		options.add_options()
			(
				"help,h",
				"show help text and exit"
			)(
				"output,o",
				po::value<std::string>(&filename),
				"file to write output to"
			)(
				"size,s",
				po::value<std::size_t>(&size),
				"number of bytes to generate"
			);
		auto vm = po::variables_map{};
		po::store(po::parse_command_line(static_cast<int>(args.size()), args.data(), options), vm);
		if (vm.count("help")) {
			std::cout.exceptions(std::ios_base::badbit);
			std::cout << "usage: huge_random_file [-s SIZE] [-o FILE]\n"
			          << "\n"
			          << "Generate a file with random data.\n"
			          << "\n"
			          << options
			          << std::endl;
			return;
		}
		po::notify(vm);
		dump_to_file(filename, size);
	}

}  // namespace /* anonymous */

int
main(int argc, char** argv)
{
	try {
		const auto args = std::vector<const char*>(argv, argv + argc);
		real_main(args);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::fprintf(stderr, "error: %s\n", e.what());
		return EXIT_FAILURE;
	}
}
