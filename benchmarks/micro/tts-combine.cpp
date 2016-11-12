#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include "lexer/token_type.hpp"
#include "lexer/token_type_set.hpp"

#include "benchmark.hpp"


namespace /* anonymous */
{

	bool benchmark(const std::vector<minijava::token_type_set>& input)
	{
		testaux::clobber_memory(input.data());
		auto tts1 = minijava::token_type_set{};
		auto tts2 = minijava::token_type_set{};
		for (const auto& tts : input) {
			if (tts.empty()) {    // 1st operation (hardly ever true)
				tts1 = tts;
				tts2 = tts;
			}
			if (tts1 == tts2) {   // 2nd operation
				tts1 |= tts;      // 3rd operation
				tts2 &= tts;      // 4th operation
			} else {
				tts1 &= tts;      // 3rd operation
				tts2 |= tts;      // 4th operation
			}
		}
		testaux::clobber_memory(&tts1);
		testaux::clobber_memory(&tts2);
		return (tts1 == tts2);
	}

	template <typename RndEngT>
	std::vector<minijava::token_type_set>
	get_input(RndEngT& engine, const std::size_t count)
	{
		const auto universe = minijava::all_token_types();
		auto dist = std::bernoulli_distribution{};
		auto inputs = std::vector<minijava::token_type_set>(count);
		for (auto& tts : inputs) {
			for (const auto& tt : universe) {
				if (dist(engine)) {
					tts.add(tt);
				}
			}
		}
		return inputs;
	}

	void real_main(int argc, char** argv)
	{
		const auto t0 = testaux::clock_type::now();
		auto setup = testaux::benchmark_setup{
			"ttts-combine",
			"Benchmark for intersection, union, comparison and empty() on 'token_type_set's."
		};
		setup.add_cmd_arg("count", "number of iterations per batch");
		if (!setup.process(argc, argv)) {
			return;
		}
		const auto count = setup.get_cmd_arg("count");
		auto engine = testaux::get_random_engine();
		const auto input = get_input(engine, count);
		auto constr = setup.get_constraints();
		if (constr.timeout.count() > 0) {
			constr.timeout -= testaux::duration_type{testaux::clock_type::now() - t0};
		}
		const auto nops = 4 * count;
		const auto absres = testaux::run_benchmark(constr, benchmark, input);
		const auto relres = testaux::result{absres.mean / nops, absres.stdev / nops, absres.n};
		testaux::print_result(relres);
	}

}  // namespace /* anonymous */


int main(int argc, char** argv)
{
	try {
		real_main(argc, argv);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "tts-combine: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
