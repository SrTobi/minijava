#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <random>
#include <utility>
#include <vector>

#include "lexer/token_type.hpp"
#include "lexer/token_type_set.hpp"

#include "testaux/benchmark.hpp"


namespace /* anonymous */
{

	constexpr auto unroll = std::size_t{16};

	minijava::token_type_set benchmark(const std::vector<minijava::token_type>& items)
	{
		testaux::clobber_memory(items.data());
		assert(items.size() % unroll == 0);
		auto tts = minijava::token_type_set{};
		auto it = std::begin(items);
		const auto n = items.size() / unroll;
		const auto m = unroll / 2;
		for (auto i = std::size_t{}; i < n; ++i) {
			for (auto j = std::size_t{}; j < m; ++j) {
				tts.add(*it++);
				tts.remove(*it++);
			}
		}
		testaux::clobber_memory(&tts);
		return tts;
	}

	template <typename RndEngT>
	std::vector<minijava::token_type>
	get_input(RndEngT& engine, const std::size_t count)
	{
		const auto universe = minijava::all_token_types();
		auto dist = std::uniform_int_distribution<std::size_t>{0, universe.size() - 1};
		auto inputs = std::vector<minijava::token_type>{};
		inputs.reserve(count);
		std::generate_n(
			std::back_inserter(inputs),
			count,
			[&engine, &dist, &universe](){ return universe[dist(engine)]; }
		);
		return inputs;
	}

	std::size_t round_to_unroll(const std::size_t count)
	{
		return count - count % unroll;
	}

	void real_main(int argc, char * * argv)
	{
		const auto t0 = testaux::clock_type::now();
		auto setup = testaux::benchmark_setup{
			"ttts-add-remove",
			"Benchmark for insertion and removal 'from token_type_set's."
		};
		setup.add_cmd_arg("count", "number of operations to perform in one batch");
		if (!setup.process(argc, argv)) {
			return;
		}
		const auto count = round_to_unroll(setup.get_cmd_arg("count"));
		auto engine = testaux::get_random_engine();
		const auto input = get_input(engine, count);
		auto constr = setup.get_constraints();
		if (constr.timeout.count() > 0) {
			constr.timeout -= testaux::duration_type{testaux::clock_type::now() - t0};
		}
		const auto absres = testaux::run_benchmark(constr, benchmark, input);
		const auto relres = testaux::result{absres.mean / count, absres.stdev / count, absres.n};
		testaux::print_result(relres);
	}

}  // namespace /* anonymous */


int main(int argc, char** argv)
{
	try {
		real_main(argc, argv);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "tts-modify: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
