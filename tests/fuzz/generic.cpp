#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>

#include <boost/program_options.hpp>

#include "exceptions.hpp"
#include "io/file_output.hpp"
#include "irg/irg.hpp"
#include "parser/ast_misc.hpp"
#include "runtime/host_cc.hpp"
#include "semantic/semantic.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/astgen.hpp"
#include "testaux/temporary_file.hpp"

namespace po = boost::program_options;


namespace /* anonymous */
{

	enum class actions
	{
		none,
		check,
		firm_create,
		firm_dump,
		firm_asm,
		firm_link,
	};

	template <typename RndEngT>
	void do_stuff(RndEngT&           engine,
	              const std::size_t  limit,
	              const bool         print,
	              const actions      action)
	{
		auto pool = minijava::symbol_pool<>{};
		auto factory = minijava::ast_factory{};
		const auto ast = testaux::generate_semantic_ast(engine, pool, factory, limit);
		if (print) {
			std::cout << *ast << std::flush;
		}
		if (action == actions::none) { return; }
		const auto seminfo = minijava::check_program(*ast, pool, factory);
		if (action == actions::check) { return; }
		auto firm = minijava::initialize_firm();
		auto ir = create_firm_ir(*firm, *ast, seminfo, "test");
		if (action == actions::firm_create) { return; }
		const testaux::temporary_directory tempdir{};
		minijava::dump_firm_ir(ir, tempdir.filename());
		if (action == actions::firm_dump) { return; }
		const auto asmfilename = tempdir.filename("test.s");
		auto asmfile = minijava::file_output{asmfilename};
		minijava::emit_x64_assembly_firm(ir, asmfile);
		asmfile.close();
		if (action == actions::firm_asm) { return; }
		const auto cc = minijava::get_default_c_compiler();
		minijava::link_runtime(cc, tempdir.filename("a.out"), asmfilename);
		if (action == actions::firm_link) { return; }
		MINIJAVA_NOT_REACHED();
	}

	void real_main(int argc, char** argv)
	{
		auto action = actions::none;
		auto theaction = std::string{};
		auto print = false;
		auto seed = 0U;
		auto limit = std::ptrdiff_t{};
		auto options = po::options_description{"Options"};
		options.add_options()
			(
				"print,p",
				po::bool_switch(&print),
				"serialize and write the program to standard output"
			)(
				"random-seed,s",
				po::value<unsigned>(&seed),
				"use explicit random seed instead of non-determinism"
			)(
				"recursion-limit,r",
				po::value<std::ptrdiff_t>(&limit)->default_value(20),
				"set limit for recursion depth"
			)(
				"action,a",
				po::value<std::string>(&theaction),
				"select action to perform"
			)(
				"help,h",
				"show help text and exit"
			);
		auto varmap = po::variables_map{};
		po::store(po::parse_command_line(argc, argv, options), varmap);
		if (varmap.count("help")) {
			std::cout << "usage: generic [-p] [-s SEED] [-r N] [-a ACTION]\n"
					  << "\n"
					  << "Generates a semantically correct random MiniJava program and runs it\n"
					  << "through various stages of the compiler.\n"
					  << "\n"
					  << options << "\n"
					  << "\n"
					  << "The following actions are available:\n"
					  << "\n"
					  << "  none          only generate the AST\n"
					  << "  check         perform semantic analysis\n"
					  << "  firm-create   create the Firm IRG\n"
					  << "  firm-dump     dump the Firm IRG\n"
					  << "  firm-asm      output assembly using Firm's backend\n"
					  << "  firm-link     create an executable using Firm's backend\n"
					  << "\n"
					  << "Actions that produce file output will do so in a temporary directory\n"
					  << "that is deleted before the program exits even in case of a failure.\n"
					  << "\n"
					  << "The exit status will be zero if and only if the compiler successfully\n"
					  << "processed the generated AST.\n"
					  << std::flush;
			return;
		}
		po::notify(varmap);
		if (limit < 0) {
			throw po::error{"Recursion limit cannot be negative"};
		}
		if (!varmap.count("random-seed")) {
			std::random_device rnddev{};
			seed = rnddev();
		}
		if (varmap.count("action")) {
			const auto actionsmap = std::map<std::string, actions>{
				{"none",        actions::none},
				{"check",       actions::check},
				{"firm-create", actions::firm_create},
				{"firm-dump",   actions::firm_dump},
				{"firm-asm",    actions::firm_asm},
				{"firm-link",   actions::firm_link},
			};
			const auto pos = actionsmap.find(theaction);
			if (pos == actionsmap.end()) {
				throw po::error{"Unknown action: " + theaction};
			}
			action = pos->second;
		}
		std::cout << "/* random seed:      " << std::setw(12) << seed  << " */\n"
				  << "/* recursion limit:  " << std::setw(12) << limit << " */\n";
		auto rndeng = std::default_random_engine(seed);
		do_stuff(rndeng, static_cast<std::size_t>(limit), print, action);
	}

}  // namespace /* anonymous */


int main(int argc, char** argv)
{
	try {
		std::cout.exceptions(std::ios_base::badbit);
		real_main(argc, argv);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "generic: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
