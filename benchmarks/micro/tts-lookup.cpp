#include <algorithm>
#include <cassert>
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

	std::size_t benchmark(const std::vector<minijava::token_type_set>& haystacks,
						  const std::vector<minijava::token_type>& needles)
	{
		testaux::clobber_memory(haystacks.data());
		testaux::clobber_memory(needles.data());
		assert(haystacks.size() == needles.size());
		assert(haystacks.size() % unroll == 0);
		auto counter = std::size_t{};
		auto haystk_it = std::begin(haystacks);
		auto needle_it = std::begin(needles);
		const auto n = haystacks.size() / unroll;
		for (auto i = std::size_t{}; i < n; ++i) {
			for (auto j = std::size_t{}; j < unroll; ++j) {
				counter += haystk_it->contains(*needle_it);
				++haystk_it;
				++needle_it;
			}
		}
		testaux::clobber_memory(&counter);
		return counter;
	}

	template <typename RndEngT>
	std::vector<minijava::token_type_set>
	get_haystacks(RndEngT& engine, const std::size_t count)
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

	template <typename RndEngT>
	std::vector<minijava::token_type>
	get_needles(RndEngT& engine, const std::size_t count)
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

	void print_inputs(const std::vector<minijava::token_type_set>& haystacks,
	                  const std::vector<minijava::token_type>& needles)
	{
		assert(haystacks.size() == needles.size());
		const auto count = haystacks.size();
		const auto universe = minijava::all_token_types();
		for (auto i = std::size_t{}; i < count; ++i) {
			const auto tts = haystacks[i];
			const auto idx = index(needles[i]);
			for (auto j = std::size_t{}; j < universe.size(); ++j) {
				if (j == idx) { std::clog << '['; }
				std::clog << tts.contains(universe[j]);
				if (j == idx) { std::clog << ']'; }
			}
			std::clog << '\n';
		}
	}

	std::size_t round_to_unroll(const std::size_t count)
	{
		return count - count % unroll;
	}

	void real_main(int argc, char** argv)
	{
		const auto t0 = testaux::clock_type::now();
		auto setup = testaux::benchmark_setup{
			"ttts-add-remove",
			"Benchmark for lookup in 'token_type_set's."
		};
		setup.add_cmd_arg("count", "number of iterations per batch");
		setup.add_cmd_flag("print", "print the sample data to standard error output");
		if (!setup.process(argc, argv)) {
			return;
		}
		const auto count = round_to_unroll(setup.get_cmd_arg("count"));
		auto engine = testaux::get_random_engine();
		const auto haystacks = get_haystacks(engine, count);
		const auto needles = get_needles(engine, count);
		if (setup.get_cmd_flag("print")) {
			print_inputs(haystacks, needles);
		}
		auto constr = setup.get_constraints();
		if (constr.timeout.count() > 0) {
			constr.timeout -= testaux::duration_type{testaux::clock_type::now() - t0};
		}
		const auto absres = testaux::run_benchmark(constr, benchmark, haystacks, needles);
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
		std::cerr << "tts-lookup: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
