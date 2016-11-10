#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <tuple>
#include <vector>

#include <boost/program_options.hpp>

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

	void real_main(const std::vector<const char *>& args)
	{
		namespace po = boost::program_options;
		const auto cons = testaux::get_constraints_from_environment();
		auto size = std::ptrdiff_t{};
		auto options = po::options_description{"Options"};
		options.add_options()
			(
				"size",
				po::value<std::ptrdiff_t>(&size)->required(),
				"number of characters to classify in one batch"
			)(
				"help",
				"show help text and exit"
			);
		auto vm = po::variables_map{};
		po::store(po::parse_command_line(static_cast<int>(args.size()), args.data(), options), vm);
		po::notify(vm);
		if (vm.count("help")) {
			std::cout << "usage: character ---size=N\n"
					  << "\n"
					  << "Run micro-benchmarks for the character classification functions.\n"
					  << "\n"
					  << options << "\n";
			return;
		}
		if (size < 1) {
			throw po::error{"The size must not be negative"};
		}
		if (cons.verbose) {
			std::clog << "Generating input data with " << size << " characters...\n";
		}
		const auto input = get_input(static_cast<std::size_t>(size));
		if (cons.verbose) {
			std::clog << "Running benchmark " << size << " ...\n";
		}
		const auto absres = testaux::run_benchmark(cons, benchmark, input);
		const auto relres = testaux::result{absres.mean / size, absres.stdev / size, absres.n};
		testaux::print_result(relres);
	}


}

int main(int argc, char * * argv)
{
	try {
		const auto args = std::vector<const char *>(argv, argv + argc);
		real_main(args);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "character: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
