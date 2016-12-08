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

#include "parser/ast_factory.hpp"
#include "parser/ast_misc.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/astgen.hpp"

namespace po = boost::program_options;


namespace /* anonymous */
{

	void do_stuff(const std::size_t limit, const unsigned seed, std::ostream& out)
	{
		auto engine = std::default_random_engine{seed};
		auto pool = minijava::symbol_pool<>{};
		auto factory = minijava::ast_factory{};
		const auto ast = testaux::generate_semantic_ast(engine, pool, factory, limit);
		const auto size = factory.id();
		out << "/* Randomly generated semantically correct MiniJava program.  */\n"
			<< "\n"
			<< "/* Random seed:             " << std::setw(12) << seed  << " */\n"
			<< "/* Maximum recursion depth: " << std::setw(12) << limit << " */\n"
			<< "/* Number of AST nodes:     " << std::setw(12) << size  << " */\n"
			<< "\n"
			<< *ast
			<< std::endl;
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
			std::cout << "usage: astgen [-s SEED] [-r N] [-o FILE]\n"
					  << "\n"
					  << "Generates a semantically correct random MiniJava program\n"
					  << "and pretty-prints it.\n"
					  << "\n"
					  << "Note that due to the way the pretty-print format is specified,\n"
					  << "the output doesn't necessarily match the generated AST.\n"
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
