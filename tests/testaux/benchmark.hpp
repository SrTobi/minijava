/**
 * @file benchmark.hpp
 *
 * @brief
 *     Utility features for writing micro-benchmarks that are compatible with
 *     the runner script.
 *
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>


namespace testaux
{

	/**
	 * @brief
	 *     Introduces a line in the code among which the compiler cannot
	 *     reorder code.
	 *
	 * Otherwise, this function is a no-op.
	 *
	 */
	inline void compiler_barrier() noexcept {
		asm volatile ("" : : : "memory");
	}

	/**
	 * @brief
	 *     Introduces a line in the code among which the compiler cannot
	 *     make assumptions about the content of the memory reacahble via `p`.
	 *
	 * Otherwise, this function is a no-op.
	 *
	 * @param p
	 *     pointer to memory to clobber
	 *
	 */
	inline void clobber_memory(const void *const p) noexcept {
		asm volatile ("" : : "rm"(p) : "memory");
	}

	/**
	 * @brief
	 *     Get a seeded random engine that is ready to use.
	 *
	 * The current implementation always seeds the engine non-deterministically
	 * but it might be changed in the future to honor user options to use an
	 * explicit seed value for reproducability.  Benchmarks should always use
	 * this function to obtain an engine.
	 *
	 * @returns
	 *     seeded random engine
	 *
	 */
	std::default_random_engine get_random_engine();

	/** @brief Clock type used for benchmarking. */
	using clock_type = std::chrono::steady_clock;

	/** @brief Duration type used for benchmarking. */
	using duration_type = std::chrono::duration<double>;

	/**
	 * @brief
	 *     Statistical result of running a benchmark.
	 *
	 */
	struct result
	{

		/** @brief Average wall-time taken by the code (non-negative). */
		duration_type mean{};

		/** @brief Standard deviation of the wall-time taken (non-negative). */
		duration_type stdev{};

		/** @brief Number of samples used to compute the statistics (at least 3). */
		std::size_t n{};

	};


	/**
	 * @brief
	 *     Exception used to indicate that a benchmark has failed and no result
	 *     could be obtained.
	 *
	 */
	struct failure : std::runtime_error
	{

		/**
		 * @brief
		 *     Creates a new exception object with the provided message.
		 *
		 * @param msg
		 *     informal explanation why the benchmark failed
		 *
		 */
		failure(const std::string& msg) : std::runtime_error{msg}
		{
		}

	};


	/**
	 * @brief
	 *     Contraints on benchmark execution.
	 *
	 */
	struct constraints
	{

		/** @brief Maximum amount of time (no limit if zero). */
		duration_type timeout{};

		/** @brief Maximum number of repetitions (no limit if zero). */
		std::size_t repetitions{};

		/** @brief Number of samples to throw away at the beginning. */
		std::size_t warmup{};

		/** @brief Fraction of best timing results to use. */
		double quantile{};

		/** @brief Desired relative standard deviation. */
		double significance{};

		/** @brief Whether to produce verbose output. */
		bool verbose{};

	};


	/**
	 * @brief
	 *     Loads benchmark constraints from the environment.
	 *
	 * This function checks the following environment variables:
	 *
	 *  - `BENCHMARK_TIMEOUT` (default: no limit)
	 *  - `BENCHMARK_REPETITIONS` (default: no limit)
	 *  - `BENCHMARK_WARMUP` (default: 0)
	 *  - `BENCHMARK_QUANTILE` (default: 1)
	 *  - `BENCHMARK_SIGNIFICANCE` (default: 20 %)
	 *  - `BENCHMARK_VERBOSE` (default: no)
	 *
	 * @returns
	 *     `constraints` initialized from environment and defaults
	 *
	 * @throws std::invalid_argument
	 *     if the environment contains bad values
	 *
	 */
	constraints get_constraints_from_environment();


	/**
	 * @brief
	 *     Runs a benchmark repetitively until the desired significance is
	 *     reached or a constraint limit is exceeded, whatever happens first.
	 *
	 * If a constraint limit is exceeded before at least three data points
	 * could be sampled, an exception is `throw`n.
	 *
	 * @tparam CallT
	 *     callable type that can be invoked with arguments `args`
	 *
	 * @tperam ArgTs
	 *     types of any arguments that should be passed to the benchmark
	 *
	 * @param c
	 *     constraints subject to which the benchmark should be run
	 *
	 * @param bench
	 *     callable object to benchmark
	 *
	 * @param args
	 *     additional arguments to pass to the callable
	 *
	 * @returns
	 *     the statistical result of running the benchmark
	 *
	 * @throws failure
	 *     if a constraint limit is hit before a result could be obtained
	 *
	 */
	template <typename CallT, typename... ArgTs>
	result run_benchmark(const constraints& c, CallT&& bench, ArgTs&&... args);

	/**
	 * @brief
	 *     Prints a result to standard output.
	 *
	 * The output format is
	 *
	 *     MEAN STDEV N
	 *
	 * where times are in seconds.  This is meant to be an easily parseable
	 * format.
	 *
	 * @param res
	 *     result to print
	 *
	 * @throws std::system_error
	 *     if the result could not be written
	 *
	 */
	void print_result(const result& res);

}  // namespace testaux


#define TESTAUX_INCLUDED_FROM_BENCHMARK_HPP
#include "benchmark.tpp"
#undef TESTAUX_INCLUDED_FROM_BENCHMARK_HPP
