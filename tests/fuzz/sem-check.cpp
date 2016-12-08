#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <random>

#include <boost/program_options.hpp>

#include "parser/ast_misc.hpp"
#include "semantic/semantic.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/astgen.hpp"

namespace po = boost::program_options;


namespace /* anonymous */
{

	template <typename RndEngT>
	void do_stuff(RndEngT& engine, const std::size_t limit, const bool print)
	{
		auto pool = minijava::symbol_pool<>{};
		auto factory = minijava::ast_factory{};
		const auto ast = testaux::generate_semantic_ast(engine, pool, factory, limit);
		if (print) {
			std::cout << *ast << std::flush;
		}
		const auto seminfo = minijava::check_program(*ast, pool, factory);
	}

	void real_main(int argc, char * * argv)
	{
		auto print = false;
		auto seed = 0U;
		auto limit = std::ptrdiff_t{};
		auto options = po::options_description{"Options"};
		options.add_options()
			(
				"print,p",
				po::bool_switch(&print),
				"serialize and write the program to standard output"
			)(
				"random-seed,s",
				po::value<unsigned>(&seed),
				"use explicit random seed instead of non-determinism"
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
			std::cout << "usage: sem-check [-p] [-s SEED] [-r N]\n"
					  << "\n"
					  << "Generates a semantically correct random MiniJava program\n"
					  << "and runs it through semantic analysis.\n"
					  << "\n"
					  << options << "\n"
					  << std::flush;
			return;
		}
		po::notify(varmap);
		if (limit < 0) {
			throw po::error{"Recursion limit cannot be negative"};
		}
		if (varmap.count("random-seed") == 0) {
			std::random_device rnddev{};
			seed = rnddev();
		}
		std::cout << "/* random seed:      " << std::setw(12) << seed  << " */\n"
				  << "/* recursion limit:  " << std::setw(12) << limit << " */\n";
		auto rndeng = std::default_random_engine(seed);
		do_stuff(rndeng, static_cast<std::size_t>(limit), print);
	}

}  // namespace /* anonymous */


int main(int argc, char * * argv)
{
	try {
		std::cout.exceptions(std::ios_base::badbit);
		real_main(argc, argv);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "sem-check: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
