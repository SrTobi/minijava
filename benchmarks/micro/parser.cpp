#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <vector>

#include "exceptions.hpp"
#include "lexer/serializer.hpp"
#include "parser/parser.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/benchmark.hpp"
#include "testaux/random_tokens.hpp"
#include "testaux/syntaxgen.hpp"


namespace /* anonymous */
{

	void benchmark(const std::vector<minijava::token>& input)
	{
		testaux::clobber_memory(input.data());
		minijava::parse_program(std::begin(input), std::end(input));
	}


	void real_main(int argc, char * * argv)
	{
		const auto t0 = testaux::clock_type::now();
		auto setup = testaux::benchmark_setup{
			"parser",
			"Benchmark for pure parser performance bypassing I/O and the lexer."
		};
		setup.add_cmd_arg("recursion-depth", "recursion depth for deriving the input");
		setup.add_cmd_flag("print", "print the sample data to standard error output");
		if (!setup.process(argc, argv)) {
			return;
		}
		const auto depth = setup.get_cmd_arg("recursion-depth");
		auto engine = testaux::get_random_engine();
		auto pool = minijava::symbol_pool<>{};
		const auto input = testaux::generate_valid_program(engine, pool, depth);
		const auto size = input.size();
		if (setup.get_cmd_flag("print")) {
			std::clog << "/* Randomly generated MiniJava program.  */\n"
					  << "/* Number of tokens:        " << std::setw(12) << size  << " */\n"
					  << "/* Maximum recursion depth: " << std::setw(12) << depth << " */\n"
					  << "\n";
			minijava::pretty_print(std::clog, std::begin(input), std::end(input));
			std::clog << std::endl;
		}
		auto constr = setup.get_constraints();
		if (constr.timeout.count() > 0) {
			constr.timeout -= testaux::duration_type{testaux::clock_type::now() - t0};
		}
		const auto absres = testaux::run_benchmark(constr, benchmark, input);
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
