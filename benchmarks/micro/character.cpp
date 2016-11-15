#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <boost/program_options.hpp>

#include "lexer/character.hpp"
#include "testaux/benchmark.hpp"


namespace /* anonymous */
{

	auto benchmark(const std::vector<int>& input, const int what)
	{
		const auto first = std::begin(input);
		const auto last = std::end(input);
		testaux::clobber_memory(input.data());
		auto result = std::distance(first, last);  // to deduce correct type
		switch (what) {
		case 's':
			{
				const auto lambda = [](const auto c){ return minijava::is_space(c); };
				result = std::count_if(first, last, lambda);
			}
			break;
		case 'd':
			{
				const auto lambda = [](const auto c){ return minijava::is_digit(c); };
				result = std::count_if(first, last, lambda);
			}
			break;
		case 'h':
			{
				const auto lambda = [](const auto c){ return minijava::is_word_head(c); };
				result = std::count_if(first, last, lambda);
			}
			break;
		case 't':
			{
				const auto lambda = [](const auto c){ return minijava::is_word_tail(c); };
				result = std::count_if(first, last, lambda);
			}
			break;
		default:
			throw std::runtime_error{"Unexpected selector: " + std::to_string(what)};
		}
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

	int get_classification_function(const testaux::benchmark_setup& setup)
	{
		using namespace std::string_literals;
		namespace po = boost::program_options;
		auto choice = 0;
		auto tally = 0;
		for (const auto& flag : {"space"s, "digit"s, "head"s, "tail"s}) {
			if (setup.get_cmd_flag(flag)) {
				++tally;
				choice = flag.front();
			}
		}
		if (tally != 1) {
			throw po::error{"Please select exactly one character classification function to benchmark"};
		}
		return choice;
	}

	void real_main(int argc, char * * argv)
	{
		const auto t0 = testaux::clock_type::now();
		auto setup = testaux::benchmark_setup{
			"character",
			"Benchmark for character classification functions."
		};
		setup.add_cmd_arg("size", "number of characters to classify in one batch");
		setup.add_cmd_flag("space", "benchmark the is_space() function");
		setup.add_cmd_flag("digit", "benchmark the is_digit() function");
		setup.add_cmd_flag("head", "benchmark the is_word_head() function");
		setup.add_cmd_flag("tail", "benchmark the is_word_tail() function");
		if (!setup.process(argc, argv)) {
			return;
		}
		const auto size = setup.get_cmd_arg("size");
		const auto what = get_classification_function(setup);
		const auto input = get_input(size);
		auto constr = setup.get_constraints();
		if (constr.timeout.count() > 0) {
			constr.timeout -= testaux::duration_type{testaux::clock_type::now() - t0};
		}
		const auto absres = testaux::run_benchmark(constr, benchmark, input, what);
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
