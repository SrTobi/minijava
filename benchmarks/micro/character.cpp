#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>

#include "lexer/character.hpp"

#include "benchmark.hpp"


namespace /* anonymous */
{

	auto benchmark(const std::vector<int>& input)
	{
		const auto first = std::begin(input);
		const auto last = std::end(input);
		testaux::clobber_memory(input.data());
		const auto n_spaces = std::count_if(first, last, minijava::is_space);
		testaux::clobber_memory(input.data());
		const auto n_digits = std::count_if(first, last, minijava::is_digit);
		testaux::clobber_memory(input.data());
		const auto n_head = std::count_if(first, last, minijava::is_word_head);
		testaux::clobber_memory(input.data());
		const auto n_tail = std::count_if(first, last, minijava::is_word_tail);
		testaux::clobber_memory(input.data());
		const auto result = std::make_tuple(n_spaces, n_digits, n_head, n_tail);
		testaux::clobber_memory(&result);
		return result;
	}

	std::vector<int> get_input(const std::size_t size)
	{
		auto input = std::vector<int>(size);
		auto rndeng = testaux::get_random_engine();
		auto rnddst = std::uniform_int_distribution<int>{-10, 300};
		std::generate(std::begin(input), std::end(input),
					  [&rndeng, &rnddst](){ return rnddst(rndeng); });
		return input;
	}

	void real_main(int argc, char * * argv)
	{
		const auto t0 = testaux::clock_type::now();
		auto setup = testaux::benchmark_setup{
			"character",
			"Benchmark for character classification functions."
		};
		setup.add_cmd_arg("size", "number of characters to classify in one batch");
		if (!setup.process(argc, argv)) {
			return;
		}
		const auto size = setup.get_cmd_arg("size");
		const auto input = get_input(size);
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
		std::cerr << "character: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
