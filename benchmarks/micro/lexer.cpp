#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iterator>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "lexer/token_iterator.hpp"
#include "lexer/token_type.hpp"
#include "symbol_pool.hpp"

#include "benchmark.hpp"
#include "random_tokens.hpp"


namespace /* anonymous */
{

	void benchmark(const std::string& input, std::vector<minijava::token>& output)
	{
		output.clear();
		testaux::clobber_memory(input.data());
		auto pool = minijava::symbol_pool<>{};
		auto lexer = minijava::make_lexer(std::begin(input), std::end(input), pool, pool);
		const auto first = minijava::token_begin(lexer);
		const auto last = minijava::token_end(lexer);
		std::copy(first, last, std::back_inserter(output));
		testaux::clobber_memory(output.data());
	}

	template <typename RdEngT>
	minijava::token_category get_random_category(RdEngT& engine)
	{
		const minijava::token_category categories[] = {
			minijava::token_category::identifier,
			minijava::token_category::literal,
			minijava::token_category::keyword,
			minijava::token_category::punctuation,
		};
		auto idxdist = std::uniform_int_distribution<std::size_t>{0, 3};
		return categories[idxdist(engine)];
	}

	std::string get_input(const std::size_t size)
	{
		auto rndeng = testaux::get_random_engine();
		auto buffer = std::ostringstream{};
		buffer << "/* " << size << " random tokens */";
		for (auto i = std::size_t{}; i < size; ++i) {
			buffer << ' ';
			switch (get_random_category(rndeng)) {
			case minijava::token_category::identifier:
				buffer << testaux::get_random_identifier(rndeng);
				break;
			case minijava::token_category::literal:
				buffer << testaux::get_random_integer_literal(rndeng);
				break;
			case minijava::token_category::keyword:
				buffer << testaux::get_random_keyword(rndeng);
				break;
			case minijava::token_category::punctuation:
				buffer << testaux::get_random_punctuation(rndeng);
				break;
			default:
				assert(false);
			}
		}
		return buffer.str();
	}

	void real_main(int argc, char * * argv)
	{
		const auto t0 = testaux::clock_type::now();
		auto setup = testaux::benchmark_setup{
			"lexer",
			"Benchmark for pure lexer performance bypassing any I/O."
		};
		setup.add_cmd_arg("size", "number of tokens to lex in one batch");
		setup.add_cmd_flag("print", "print the sample data to standard error output");
		if (!setup.process(argc, argv)) {
			return;
		}
		const auto size = setup.get_cmd_arg("size");
		const auto input = get_input(size);
		if (setup.get_cmd_flag("print")) {
			std::clog << input << '\n';
		}
		auto output = std::vector<minijava::token>{};
		output.reserve(size + 1);  // +1 for EOF
		auto constr = setup.get_constraints();
		if (constr.timeout.count() > 0) {
			constr.timeout -= testaux::duration_type{testaux::clock_type::now() - t0};
		}
		const auto absres = testaux::run_benchmark(constr, benchmark, input, output);
		const auto relres = testaux::result{absres.mean / size, absres.stdev / size, absres.n};
		testaux::print_result(relres);
	}

}  // namespace /* anonymous */


int main(int argc, char * * argv)
{
	try {
		real_main(argc, argv);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "lexer: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
