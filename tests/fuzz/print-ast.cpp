#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>

#include <boost/program_options.hpp>

#include "exceptions.hpp"
#include "lexer/lexer.hpp"
#include "lexer/serializer.hpp"
#include "lexer/token_iterator.hpp"
#include "parser/ast_misc.hpp"
#include "parser/parser.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/syntaxgen.hpp"

namespace po = boost::program_options;


namespace /* anonymous */
{

	auto parse_and_print(const std::vector<minijava::token>& tokens)
	{
		auto ast = minijava::parse_program(std::begin(tokens), std::end(tokens));
		auto pretty = to_text(*ast);
		return std::make_pair(std::move(ast), std::move(pretty));
	}

	auto lex_parse_and_print(const std::string& text, minijava::symbol_pool<>& pool)
	{
		auto lex = minijava::make_lexer(std::begin(text), std::end(text), pool, pool);
		auto ast = minijava::parse_program(minijava::token_begin(lex), minijava::token_end(lex));
		auto pretty = to_text(*ast);
		return std::make_pair(std::move(ast), std::move(pretty));
	}

	template <typename RndEngT>
	void do_stuff(RndEngT& engine, const std::size_t limit, bool print,
	              const std::string& out1st, const std::string& out2nd)
	{
		auto pool = minijava::symbol_pool<>{};
		const auto tokens = testaux::generate_valid_program(engine, pool, limit);
		if (print) {
			minijava::pretty_print(std::cout, std::begin(tokens), std::end(tokens));
			std::cout << std::endl;
		}
		auto one = parse_and_print(tokens);
		auto two = lex_parse_and_print(one.second, pool);
		if (!out1st.empty()) {
			std::ofstream ostr{out1st};
			ostr.exceptions(std::ios_base::badbit);
			ostr << one.second << std::flush;
		}
		if (!out2nd.empty()) {
			std::ofstream ostr{out2nd};
			ostr.exceptions(std::ios_base::badbit);
			ostr << two.second << std::flush;
		}
		if (one.second != two.second) {
			throw std::runtime_error{"Serialized texts differ"};
		}
	}

	void real_main(int argc, char** argv)
	{
		auto print = false;
		auto seed = 0U;
		auto limit = std::ptrdiff_t{};
		auto out1st = std::string{};
		auto out2nd = std::string{};
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
				"save-1st,1",
				po::value<std::string>(&out1st),
				"save the 1st pretty-printed AST to the given file"
			)(
				"save-2nd,2",
				po::value<std::string>(&out2nd),
				"save the 2nd pretty-printed AST to the given file"
			)(
				"help,h",
				"show help text and exit"
			);
		auto varmap = po::variables_map{};
		po::store(po::parse_command_line(argc, argv, options), varmap);
		if (varmap.count("help")) {
			std::cout << "usage: print-ast [-p] [-s SEED] [-r N]\n"
					  << "\n"
					  << "Generates a syntactically correct random MiniJava program\n"
					  << "and feeds its token stream into the parser.  Then passes\n"
					  << "the generated AST to the pretty-printer.  Next tokenizes\n"
					  << "the generated text and parses it again.  Finally, pretty-\n"
					  << "prints the new AST.  If the results from the first and\n"
					  << "second round differ, reports an error.\n"
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
		do_stuff(rndeng, static_cast<std::size_t>(limit), print, out1st, out2nd);
	}

}  // namespace /* anonymous */


int main(int argc, char * * argv)
{
	try {
		std::cout.exceptions(std::ios_base::badbit);
		real_main(argc, argv);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "print-ast: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
