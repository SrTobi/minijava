#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <utility>

#include <boost/program_options.hpp>

#include "exceptions.hpp"
#include "lexer/serializer.hpp"
#include "parser/parser.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/ast_id_checker.hpp"
#include "testaux/syntaxgen.hpp"

namespace po = boost::program_options;


namespace /* anonymous */
{

	template <typename RndEngT>
	void do_stuff(RndEngT& engine, const std::size_t limit, bool print)
	{
		auto pool = minijava::symbol_pool<>{};
		const auto tokens = testaux::generate_valid_program(engine, pool, limit);
		std::cout << "/* number of tokens: " << std::setw(12) << tokens.size() << " */\n";
		if (print) {
			minijava::pretty_print(std::cout, std::begin(tokens), std::end(tokens));
			std::cout << std::endl;
		}
		const auto ast = minijava::parse_program(std::begin(tokens), std::end(tokens));
		testaux::check_ids_strict(*ast);
	}

	void real_main(int argc, char** argv)
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
			std::cout << "usage: parsetest [-p] [-s SEED] [-r N]\n"
					  << "\n"
					  << "Generates a syntactically correct random MiniJava program\n"
					  << "and feeds its token stream into the parser.\n"
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
		std::cerr << "parsetest: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
