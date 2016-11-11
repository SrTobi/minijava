#include "benchmark.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;


namespace testaux
{

	namespace /* anonymous */
	{

		template <typename TargetT>
		TargetT my_lexical_cast(const std::string& envval,
								const std::string& envvar,
								const std::string& invalid)
		{
			try {
				return boost::lexical_cast<TargetT>(envval);
			} catch (const boost::bad_lexical_cast&) {
				throw boost::program_options::error{envvar + ": " + invalid + ": " + envval};
			}
		}

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
			const auto raw = my_lexical_cast<double>(envval, envvar, invalid);
			if (predicate(raw)) {
				return raw;
			}
			throw po::error{envvar + ": " + invalid + ": " + envval};
		}

		double raw_get_real_positive(const std::string& envvar, const double unset)
		{
			const auto pred = [](auto x){ return std::isfinite(x) && (x > 0.0); };
			return raw_get_real(envvar, unset, pred, "A positive real is required");
		}

		duration_type get_timeout(const std::string& envvar)
		{
			const auto raw = raw_get_real_positive(envvar, 0.0);
			return duration_type{raw};
		}

		double get_significance(const std::string& envvar)
		{
			return raw_get_real_positive(envvar, 0.20);
		}

		double get_quantile(const std::string& envvar)
		{
			const auto pred = [](auto x){ return std::isfinite(x) && (x > 0.0) && (x <= 1.0); };
			return raw_get_real(envvar, 1.0, pred, "A real in the interval (0, 1] is required");
		}

		std::size_t get_count(const std::string& envvar)
		{
			const auto invalid = "A non-negative integer is required";
			const auto envval = std::getenv(envvar.c_str());
			if (envval == nullptr) {
				return 0;
			}
			const auto n = my_lexical_cast<std::ptrdiff_t>(envval, envvar, invalid);
			if (n < 0) {
				throw po::error{envvar + ": " + invalid + ": " + envval};
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
		if (!std::isfinite(m) || (m < 0.0) || !std::isfinite(s) || (s < 0.0) || (n == 0)) {
			throw std::invalid_argument{"Obtained garbage results"};
		}
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

		void print_constraints(const constraints& c)
		{
			if (c.timeout.count() > 0.0) {
				std::fprintf(stderr, "timeout:       %16.6f s\n", c.timeout.count());
			} else {
				std::fprintf(stderr, "timeout:       %16s\n", "none");
			}
			if (c.repetitions > 0) {
				std::fprintf(stderr, "repetitions:   %16zu\n", c.repetitions);
			} else {
				std::fprintf(stderr, "repetitions:   %16s\n", "none");
			}
			std::fprintf(stderr, "warmup:        %16zu\n", c.warmup);
			std::fprintf(stderr, "quantile:      %16.6f\n", c.quantile);
			std::fprintf(stderr, "significance:  %16.6f\n", c.significance);
			std::fprintf(stderr, "verbose:       %16s\n", c.verbose ? "yes" : "no");
		}

	}  // namespace detail

	namespace /* anonymous */
	{

		bool is_special_cmd_arg(const std::string& name)
		{
			static const std::string special[] = {
				"help",
				"version",
				"verbose",
				"timeout",
				"repetitions",
				"warmup",
				"quantile",
				"significance",
			};
			return (std::find(std::begin(special), std::end(special), name) != std::end(special));
		}

		void update_constraints_from_cmd_args(constraints& constr, po::variables_map& varmap)
		{
			if (varmap.count("verbose")) {
				constr.verbose = true;
			}
			if (varmap.count("timeout")) {
				const auto value = varmap["timeout"].as<double>();
				if (value <= 0.0) {
					throw po::error{"Timeout must be a positive real"};
				}
				constr.timeout = duration_type{value};
			}
			if (varmap.count("repetitions")) {
				const auto value = varmap["repetitions"].as<std::ptrdiff_t>();
				if (value < 0) {
					throw po::error{"Repetitions must be e non-negative integer"};
				}
				constr.repetitions = static_cast<std::size_t>(value);
			}
			if (varmap.count("warmup")) {
				const auto value = varmap["warmup"].as<std::ptrdiff_t>();
				if (value < 0) {
					throw po::error{"Warmup must be e non-negative integer"};
				}
				constr.warmup = static_cast<std::size_t>(value);
			}
			if (varmap.count("quantile")) {
				const auto value = varmap["quantile"].as<double>();
				if ((value <= 0.0) || (value > 1.0)) {
					throw po::error{"Quantile must be a real in the interval (0, 1]"};
				}
				constr.quantile = value;
			}
			if (varmap.count("significance")) {
				const auto value = varmap["significance"].as<double>();
				if (value <= 0.0) {
					throw po::error{"Significance must be a positive real"};
				}
				constr.significance = value;
			}
		}


	}  // namespace /* anonymous */

	benchmark_setup::benchmark_setup(const std::string& name, const std::string& description)
		: _name{name}, _description{description}
	{
	}

	void benchmark_setup::add_cmd_arg(const std::string& name, const std::string& description)
	{
		if (!is_special_cmd_arg(name)) {
			if (!_cmd_flags.count(name)) {
				if (_cmd_args.insert({name, description}).second) {
					return;
				}
			}
		}
		throw std::invalid_argument{"Name clash for command-line argument: --" + name};
	}

	void benchmark_setup::add_cmd_flag(const std::string& name, const std::string& description)
	{
		if (!is_special_cmd_arg(name)) {
			if (!_cmd_args.count(name)) {
				if (_cmd_flags.insert({name, description}).second) {
					return;
				}
			}
		}
		throw std::invalid_argument{"Name clash for command-line flag: --" + name};
	}

	bool benchmark_setup::process(int argc, char * * argv)
	{
		_constraints = get_constraints_from_environment();
		auto generic = po::options_description{"Generic Options"};
		generic.add_options()
			("help", "show help text and exit")
			("version", "show version text and exit");
		auto benchmark = po::options_description{"General Options for all Benchmarks"};
		benchmark.add_options()
			("timeout", po::value<double>(), "timeout in seconds")
			("repetitions", po::value<std::ptrdiff_t>(), "maximum number of repetitions")
			("warmup", po::value<std::ptrdiff_t>(), "number of initial samples to throw away")
			("quantile", po::value<double>(), "fraction of (best) samples to use")
			("significance", po::value<double>(), "desired relative standard deviation")
			("verbose", "print status messages to standard error output");
		auto specific = po::options_description{"Specific Options for this Benchmark"};
		for (const auto& kv : _cmd_args) {
			specific.add_options() (kv.first.c_str(), po::value<std::ptrdiff_t>()->required(), kv.second.c_str());
		}
		for (const auto& kv : _cmd_flags) {
			specific.add_options()(kv.first.c_str(), kv.second.c_str());
		}
		auto options = po::options_description{};
		options.add(specific).add(benchmark).add(generic);
		auto varmap = po::variables_map{};
		po::store(po::parse_command_line(argc, argv, options), varmap);
		std::cout.exceptions(std::ios_base::badbit);
		if (varmap.count("help")) {
			std::cout << _name << " -- " << _description << "\n\n"
					  << specific << "\n"
					  << benchmark << "\n"
					  << generic << "\n"
					  << std::flush;
			return false;
		}
		if (varmap.count("version")) {
			std::cout << _name << " -- " << _description << std::endl;
			return false;
		}
		po::notify(varmap);
		update_constraints_from_cmd_args(_constraints, varmap);
		for (const auto& kv : _cmd_args) {
			const auto value = varmap[kv.first].as<std::ptrdiff_t>();
			if (value < 0) {
				throw po::error{"A non-negative value is required for option --" + kv.first};
			}
			_cmd_arg_vals[kv.first] = static_cast<std::size_t>(value);
		}
		for (const auto& kv : _cmd_flags) {
			if (varmap.count(kv.first)) {
				_cmd_flag_vals.insert(kv.first);
			}
		}
		return true;
	}

	std::size_t benchmark_setup::get_cmd_arg(const std::string& name) const
	{
		if (_cmd_args.count(name) == 0) {
			throw std::invalid_argument{"No such command-line argument: --" + name};
		}
		return _cmd_arg_vals.at(name);
	}

	bool benchmark_setup::get_cmd_flag(const std::string& name) const
	{
		if (_cmd_flags.count(name) == 0) {
			throw std::invalid_argument{"No such command-line flag: --" + name};
		}
		return (_cmd_flag_vals.count(name) > 0);
	}

	const constraints& benchmark_setup::get_constraints() const noexcept
	{
		return _constraints;
	}

}  // namespace testaux
