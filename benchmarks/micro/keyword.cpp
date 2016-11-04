#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#include <boost/program_options.hpp>

#include "lexer/keyword.hpp"

#include "benchmark.hpp"


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

	std::string get_random_identifier(std::default_random_engine& engine)
	{
		using namespace std::string_literals;
		static const auto headchars = "_"s
			+ "ABCDEFGHILKLMNOPQRSTUVWXYZ"
			+ "abcdefghijklmnopqrstuvwxyz";
		static const auto tailchars = headchars + "0123456789";
		static auto lendst = std::uniform_int_distribution<std::size_t>{1, 100};
		static auto headdist = std::uniform_int_distribution<std::size_t>{0, headchars.size() - 1};
		static auto taildist = std::uniform_int_distribution<std::size_t>{0, tailchars.size() - 1};
		const auto length = lendst(engine);
		auto ident = std::string{};
		ident.reserve(length);
		ident.push_back(headchars[headdist(engine)]);
		while (ident.length() < length) {
			ident.push_back(tailchars[taildist(engine)]);
		}
		return ident;
	}

	std::string get_random_keyword(std::default_random_engine& engine)
	{
		static auto idxdst = std::uniform_int_distribution<std::size_t>{
			0,
			minijava::total_token_type_count - 1
		};
		while (true) {
			const auto idx = idxdst(engine);
			const auto tt = minijava::all_token_types()[idx];
			if (category(tt) == minijava::token_category::keyword) {
				return name(tt);
			}
		}
	}

	std::string get_random_word(std::default_random_engine& engine)
	{
		static auto keydst = std::bernoulli_distribution{0.5};
		return keydst(engine)
			? get_random_keyword(engine)
			: get_random_identifier(engine);
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
				"number of words to classify in one batch"
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
					  << "Run micro-benchmarks for keyword classification.\n"
					  << "\n"
					  << options << "\n";
			return;
		}
		if (size < 1) {
			throw po::error{"The size must not be negative"};
		}
		if (cons.verbose) {
			std::clog << "Generating input data with " << size << " words...\n";
		}
		const auto usize = static_cast<std::size_t>(size);
		const auto input = get_input(usize);
		auto output = std::vector<minijava::token_type>(usize);
		if (cons.verbose) {
			std::clog << "Running benchmark " << size << " ...\n";
		}
		const auto absres = testaux::run_benchmark(cons, benchmark, input, output);
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
