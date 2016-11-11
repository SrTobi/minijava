// I re-wrote this input generator as a C++ program because the equivalent
// Python script took more than a minute to generate 1 000 000 identifiers
// between 1 and 101 characters long while the actual benchmark only takes less
// than two seconds to process that input.  This program generates the same
// input in less than one second.

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <system_error>

#include <boost/program_options.hpp>

#include "random_tokens.hpp"


namespace /* anonymous */
{

	void dump_ids(const std::size_t count, const std::size_t minlen, const std::size_t maxlen)
	{
		auto rndeng = std::default_random_engine{std::random_device{}()};
		for (auto i = std::size_t{}; i < count; ++i) {
			const auto word = testaux::get_random_identifier(rndeng, minlen, maxlen);
			if (std::puts(word.c_str()) < 0) {
				const auto ec = std::error_code{errno, std::system_category()};
				throw std::system_error{ec, "Cannot write data to file"};
			}
		}
	}

	void real_main(const std::vector<const char*>& args)
	{
		namespace po = boost::program_options;
		auto count = std::ptrdiff_t{};
		auto minlen = std::ptrdiff_t{};
		auto maxlen = std::ptrdiff_t{};
		auto options = po::options_description{"Options"};
		options.add_options()
			(
				"help",
				"show help text and exit"
			)(
				"count",
				po::value<std::ptrdiff_t>(&count)->default_value(100),
				"number of identifiers"
			)(
				"min-length",
				po::value<std::ptrdiff_t>(&minlen)->default_value(1),
				"minimum length of the generated identifiers"
			)(
				"max-length",
				po::value<std::ptrdiff_t>(&maxlen)->default_value(128),
				"maximum length of the generated identifiers"
			);
		auto vm = po::variables_map{};
		po::store(po::parse_command_line(static_cast<int>(args.size()), args.data(), options), vm);
		if (vm.count("help")) {
			std::cout.exceptions(std::ios_base::badbit);
			std::cout << "usage: many_different_ids --count=N --min-length=N --max-length=N\n"
					  << "\n"
					  << "Generate a file with many random identifiers\n"
					  << "\n"
					  << options
					  << std::endl;
			return;
		}
		po::notify(vm);
		if (count < 0) {
			throw po::error{"Please select count >= 0 or it's not gonna work"};
		}
		if ((minlen < 1) || (maxlen < minlen)) {
			throw po::error{"Please select 1 <= min-length <= max_length or it's not gonna work"};
		}
		dump_ids(
			static_cast<std::size_t>(count),
			static_cast<std::size_t>(minlen),
			static_cast<std::size_t>(maxlen)
		);
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
