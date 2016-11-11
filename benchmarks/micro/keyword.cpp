#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include "lexer/keyword.hpp"

#include "benchmark.hpp"
#include "random_tokens.hpp"


namespace /* anonymous */
{

	void benchmark(const std::vector<std::string>& input,
				   std::vector<minijava::token_type>& output)
	{
		output.clear();
		testaux::clobber_memory(input.data());
		for (const auto& w : input) {
			output.push_back(minijava::classify_word(w));
		}
		testaux::clobber_memory(output.data());
	}

	std::string get_random_word(std::default_random_engine& engine)
	{
		static auto keydst = std::bernoulli_distribution{0.5};
		return keydst(engine)
			? std::string{name(testaux::get_random_keyword(engine))}
			: testaux::get_random_identifier(engine);
	}

	std::vector<std::string> get_input(const std::size_t size)
	{
		auto engine = testaux::get_random_engine();
		auto input = std::vector<std::string>();
		input.reserve(size);
		while (input.size() < size) {
			input.push_back(get_random_word(engine));
		}
		return input;
	}

	void real_main(int argc, char * * argv)
	{
		const auto t0 = testaux::clock_type::now();
		auto setup = testaux::benchmark_setup{
			"keyword",
			"Benchmark for keyword / identifier classification."
		};
		setup.add_cmd_arg("size", "number of words to classify in one batch");
		setup.add_cmd_flag("print", "print the sample data to standard error output");
		if (!setup.process(argc, argv)) {
			return;
		}
		const auto size = setup.get_cmd_arg("size");
		const auto input = get_input(size);
		if (setup.get_cmd_flag("print")) {
			for (const auto& word : input) {
				std::clog << word << '\n';
			}
		}
		auto output = std::vector<minijava::token_type>{};
		output.reserve(size);
		auto constr = setup.get_constraints();
		if (constr.timeout.count() > 0) {
			constr.timeout -= testaux::duration_type{testaux::clock_type::now() - t0};
		}
		const auto absres = testaux::run_benchmark(constr, benchmark, input, output);
		const auto relres = testaux::result{absres.mean / size, absres.stdev / size, absres.n};
		testaux::print_result(relres);
	}

}

int main(int argc, char * * argv)
{
	try {
		real_main(argc, argv);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "character: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
