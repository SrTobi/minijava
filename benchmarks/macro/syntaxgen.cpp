#include <cstddef>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <utility>

#include <boost/program_options.hpp>

#include "lexer/serializer.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/syntaxgen.hpp"

namespace po = boost::program_options;


namespace /* anonymous */
{

	void do_stuff(const std::size_t limit, const unsigned seed, std::ostream& out)
	{
		auto engine = std::default_random_engine{seed};
		auto pool = minijava::symbol_pool<>{};
		const auto input = testaux::generate_valid_program(engine, pool, limit);
		const auto size = input.size();
		out << "/* Randomly generated syntactically correct MiniJava program.  */\n"
			<< "\n"
			<< "/* Random seed:             " << std::setw(12) << seed  << " */\n"
			<< "/* Maximum recursion depth: " << std::setw(12) << limit << " */\n"
			<< "/* Number of tokens:        " << std::setw(12) << size  << " */\n"
			<< "\n";
		minijava::pretty_print(out, std::begin(input), std::end(input));
		out << std::endl;
	}

	void real_main(int argc, char** argv)
	{
		auto output = std::string{"-"};
		auto seed = 0u;
		auto limit = std::ptrdiff_t{};
		auto options = po::options_description{"Options"};
		options.add_options()
			(
				"output,o",
				po::value<std::string>(&output),
				"redirect output to file"
			)(
				"random-seed,s",
				po::value<unsigned>(&seed),
				"use explicit deterministic random seed"
			)(
				"recursion-limit,r",
				po::value<std::ptrdiff_t>(&limit)->default_value(20),
				"set limit for recursion depth"
			)(
				"help,h",
				"show help text and exit"
			);
		auto varmap = po::variables_map{};
		po::store(po::parse_command_line(argc, argv, options), varmap);
		if (varmap.count("help")) {
			std::cout << "usage: syntaxgen [-s SEED] [-r N] [-o FILE]\n"
					  << "\n"
					  << "Generates a syntactically correct random MiniJava program\n"
					  << "and pretty-prints it.\n"
					  << "\n"
					  << options << "\n"
					  << std::flush;
			return;
		}
		po::notify(varmap);
		if (limit < 1) {
			throw po::error{"Recursion limit must be positive"};
		}
		if (varmap.count("random-seed") == 0) {
			std::random_device rnddev{};
			seed = rnddev();
		}
		if (output == "-") {
			do_stuff(static_cast<std::size_t>(limit), seed, std::cout);
		} else {
			std::ofstream ostr {output};
			ostr.exceptions(std::ios_base::failbit);
			do_stuff(static_cast<std::size_t>(limit), seed, ostr);
		}
	}

}  // namespace /* anonymous */


int main(int argc, char** argv)
{
	try {
		real_main(argc, argv);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::fprintf(stderr, "error: %s\n", e.what());
		return EXIT_FAILURE;
	}
}
