#include "benchmark.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <system_error>

#include <boost/lexical_cast.hpp>


namespace testaux
{

	namespace /* anonymous */
	{

		template <typename PredT>
		double raw_get_real(const std::string& envvar,
							const double unset,
							const PredT& predicate,
							const std::string& invalid)
		{
			const auto envval = std::getenv(envvar.c_str());
			if (envval == nullptr) {
				return unset;
			}
			const auto raw = boost::lexical_cast<double>(envval);
			if (predicate(raw)) {
				return raw;
			}
			throw std::invalid_argument{envvar + ": " + invalid};
		}

		double raw_get_real_non_negative(const std::string& envvar, const double unset)
		{
			const auto pred = [](auto x){ return std::isfinite(x) && (x >= 0.0); };
			return raw_get_real(envvar, unset, pred, "A non-negative real is required");
		}

		duration_type get_timeout(const std::string& envvar)
		{
			const auto raw = raw_get_real_non_negative(envvar, 0.0);
			return duration_type{raw};
		}

		double get_significance(const std::string& envvar)
		{
			return raw_get_real_non_negative(envvar, 0.20);
		}

		double get_quantile(const std::string& envvar)
		{
			const auto pred = [](auto x){ return std::isfinite(x) && (x > 0.0) && (x <= 1.0); };
			return raw_get_real(envvar, 1.0, pred, "A real in the interval (0, 1] is required");
		}

		std::size_t get_count(const std::string& envvar)
		{
			const auto envval = std::getenv(envvar.c_str());
			if (envval == nullptr) {
				return 0;
			}
			const auto n = boost::lexical_cast<std::ptrdiff_t>(envval);
			if (n < 0) {
				throw std::invalid_argument{envvar + ": A non-negative integer is required"};
			}
			return static_cast<std::size_t>(n);
		}

		bool get_bool(const std::string& envvar)
		{
			const auto raw = get_count(envvar);
			return (raw > 0);
		}

	}  // namespace /* anonymous */


	constraints get_constraints_from_environment()
	{
		auto c = constraints{};
		c.timeout = get_timeout("BENCHMARK_TIMEOUT");
		c.repetitions = get_count("BENCHMARK_REPETITIONS");
		c.warmup = get_count("BENCHMARK_WARMUP");
		c.quantile = get_quantile("BENCHMARK_QUANTILE");
		c.significance = get_significance("BENCHMARK_SIGNIFICANCE");
		c.verbose = get_bool("BENCHMARK_VERBOSE");
		return c;
	}

	std::default_random_engine get_random_engine()
	{
		std::random_device device{};
		std::default_random_engine engine{device()};
		return engine;
	}

	void print_result(const result& res)
	{
		const auto m = res.mean.count();
		const auto s = res.stdev.count();
		const auto n = res.n;
		if ((std::printf("%18.8E  %18.8E  %18zu\n", m, s, n) < 0) || (std::fflush(stdout) < 0)) {
			const auto ec = std::error_code{errno, std::system_category()};
			throw std::system_error{ec, "Cannot write data to file"};
		}
	}

	namespace detail
	{

		void print_verbose_progress(const std::size_t i, const duration_type t)
		{
			std::fprintf(stderr, "%18zu  %18.8E s\n", i, t.count());
		}

	}  // namespace detail

}  // namespace testaux
