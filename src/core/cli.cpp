#include "cli.hpp"

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/program_options.hpp>

#include "exceptions.hpp"
#include "global.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token_iterator.hpp"
#include "parser/parser.hpp"
#include "parser/pretty_printer.hpp"
#include "semantic/ref_type_analysis.hpp"
#include "semantic/symbol_def.hpp"
#include "symbol/symbol_pool.hpp"
#include "system/system.hpp"


namespace algo = boost::algorithm;
namespace po = boost::program_options;

namespace minijava
{

	namespace /* anonymous */
	{

		// Used to select the stage at which the compilation should be
		// intercepted.
		enum class compilation_stage
		{
			input = 1,
			lexer = 2,
			parser = 3,
			print_ast = 4,
			semantic = 5,
		};


		// Parsed command-line options.
		struct program_setup
		{
			// Stage at which the compilation should be intercepted.
			compilation_stage stage{};

			// File-name of the input file (may be `-` to read from stdin).
			std::string input{};

			// File-name of the output file (may be `-` to write to stdout).
			std::string output{};
		};


		// Checks that at most one option from `group` is set in `varmap` and
		// `throw`s a `po::error` with an appropriate message otherwise.
		void check_mutex_option_group(const po::options_description& group,
		                              const po::variables_map& varmap)
		{
			using namespace std::string_literals;
			auto seen = std::vector<std::string>{};
			for (const auto& option : group.options()) {
				const auto key = option->key("");
				if (varmap.count(key) && !varmap[key].defaulted()) {
					const auto style = 1;  // determined by trial and error
					seen.push_back(option->canonical_display_name(style));
				}
			}
			if (seen.size() > 1) {
				const auto namelist = algo::join(seen, " "s);
				throw po::error{"Mutually exclusive options: " + namelist};
			}
		}


		// Parses the command-line arguments in `args` (which must include the
		// executable path, or any dummy string, as its first element) and sets
		// the values of `setup` accordingly.  If the command-line was not
		// valid, a `po::error` is `throw`n and the value of `setup` is
		// unspecified.  If the `--help` or `--version` option was seen, the
		// corresponding text is written to `out` and `false` is `return`ed and
		// the value of `setup` is unspecified.  Otherwise, `true` is
		// `return`ed and `setup` contains the parsed values.
		bool parse_cmd_options(const std::vector<const char *>& args,
		                       std::ostream& out, program_setup& setup)
		{
			auto generic = po::options_description{"Generic Options"};
			generic.add_options()
				("help", "show help text and exit")
				("version", "show version text and exit");
			auto interception = po::options_description{"Intercepting the Compilation at Specific Stages"};
			interception.add_options()
				("echo", "stop after the input stage and output the source file verbatim")
				("lextest", "stop after lexical analysis and output a token sequence")
				("parsetest", "stop after parsing and reporting any syntax errors")
				("print-ast", "stop after parsing and print the parsed ast")
				("check", "stop after analysis and reporting any semantic errors");
			auto other = po::options_description{"Other Options"};
			other.add_options()
				("output", po::value<std::string>(&setup.output)->default_value("-"), "redirect output to file");
			auto inputfiles = po::options_description{"Input Files"};
			inputfiles.add_options()
				("input", po::value<std::string>(&setup.input)->default_value("-"), "");
			auto options = po::options_description{};
			options.add(generic).add(interception).add(other).add(inputfiles);
			auto positional = po::positional_options_description{};
			positional.add("input", 1);
			auto varmap = po::variables_map{};
			const int argc = static_cast<int>(args.size());  // safe cast
			po::store(po::command_line_parser(argc, args.data())
			         .options(options).positional(positional).run(), varmap);
			po::notify(varmap);
			check_mutex_option_group(interception, varmap);
			if (varmap.count("help")) {
				out << "usage: " << MINIJAVA_PROJECT_NAME << " [OPTIONS] FILE\n"
				    << '\n' << generic
				    << '\n' << interception
				    << '\n' << other
				    << '\n'
				    << "Anywhere a file name is expected, '-' can be used to refer to the standard\n"
				    << "input or output stream respectively\n";
				return false;
			}
			if (varmap.count("version")) {
				out << MINIJAVA_PROJECT_NAME << " " << MINIJAVA_PROJECT_VERSION << "\n"
				    << "Copyright (C) 2016 T. Kahlert, P.J. Serrer, M. Baumann and M. Klammler\n"
				    << "This is free software; see the source for copying conditions.  There is NO\n"
				    << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
				return false;
			}
			// If you know a better solution for this, please share your
			// knowledge with us: https://stackoverflow.com/q/40305153/
			if (varmap.count("echo")) {
				setup.stage = compilation_stage::input;
			}
			if (varmap.count("lextest")) {
				setup.stage = compilation_stage::lexer;
			}
			if (varmap.count("parsetest")) {
				setup.stage = compilation_stage::parser;
			}
			if (varmap.count("print-ast")) {
				setup.stage = compilation_stage::print_ast;
			}
			if (varmap.count("check")) {
				setup.stage = compilation_stage::semantic;
			}
			return true;
		}


		// Runs the compiler reading input from `istr`, writing output to
		// `ostr` and optionally intercepting compilation at `stage`.
		void run_compiler(std::istream& istr, std::ostream& ostr,
		                  const compilation_stage stage)
		{
			const auto infirst = std::istreambuf_iterator<char>{istr};
			const auto inlast = std::istreambuf_iterator<char>{};
			if (stage == compilation_stage::input) {
				std::copy(infirst, inlast, std::ostreambuf_iterator<char>{ostr});
				return;
			}
			auto pool = symbol_pool<>{};  // TODO: Use an appropriate allocator
			auto lex = make_lexer(infirst, inlast, pool, pool);
			const auto tokfirst = token_begin(lex);
			const auto toklast = token_end(lex);
			if (stage == compilation_stage::lexer) {
				std::copy(tokfirst, toklast, std::ostream_iterator<token>{ostr, "\n"});
				return;
			}
			auto ast = parse_program(tokfirst, toklast);
			if (stage == compilation_stage::parser) {
				return;
			}

			if (stage == compilation_stage::print_ast) {
				ast::pretty_printer printer_visitor(ostr);
				ast->accept(printer_visitor);
				return;
			}
			if (stage == compilation_stage::semantic) {
				auto defa = semantic::def_annotations{};
				auto typs = semantic::extract_typesystem(*ast, defa, pool);
				auto syst = semantic::buildins::register_system(typs, pool);
				semantic::analyse_program(*ast, {{pool.normalize("System"), syst}}, typs, defa);
				return;
			}
			// If we get until here, we have a problem...
			throw not_implemented_error{"The rest of the compiler has yet to be written"};
		}


		// Applies a final check to the input stream `istr` and `throw`s an
		// exception if needed.
		void finalize_stream(std::istream& istr)
		{
			// Don't test `!istr` because `istr.eof()` is perfectly reasonable.
			if (istr.bad()) {
				throw std::ios_base::failure{"Input not readable"};
			}
		}


		// Flushes the output stream `ostr`, applies a final check to it and
		// `throw`s an exception if needed.
		void finalize_stream(std::ostream& ostr)
		{
			if (!ostr.flush()) {
				throw std::ios_base::failure{"Output not writeable"};
			}
		}


		// Calls `run_compiler` with the provided argumetns and finally checks
		// the streams whether input was fully consumed, is not in error and
		// output is good; if not, `throw`s an exception.
		void run_compiler_with_streams(std::istream& istr,
		                               std::ostream& ostr,
		                               const compilation_stage stage)
		{
			istr.exceptions(std::ios_base::badbit);
			ostr.exceptions(std::ios_base::failbit);
			run_compiler(istr, ostr, stage);
			finalize_stream(istr);
			finalize_stream(ostr);
		}


		// Checks the environment variable `MINIJAVA_STACK_LIMIT` and
		// `return`s its value.  If the variable is set in the environment and
		// has a valid value, its value is `return`ed.  Otherwise, 0 (which is
		// not a valid value) is `return`ed.  If it is set to an invalid value,
		// a warning is printed to `err`.
		std::ptrdiff_t get_stack_limit(std::ostream& err)
		{
			const auto envval = std::getenv(MINIJAVA_ENVVAR_STACK_LIMIT);
			if (envval == nullptr) {
				return 0;
			}
			const auto text = std::string{envval};
			if (boost::iequals("DEFAULT", text)) {
				return 0;
			}
			if (boost::iequals("NONE", text)) {
				return -1;
			}
			try {
				const auto value = boost::lexical_cast<std::ptrdiff_t>(text);
				if (value > 0) {
					return value;
				}
			} catch (const boost::bad_lexical_cast&) { /* fall through */ }
			// TODO: Once we have a logging facility, we should use it here
			// instead of printing directly.
			err << "warning: "
			    << MINIJAVA_ENVVAR_STACK_LIMIT
			    << ": not a valid stack size in bytes: "
			    << envval
			    << std::endl;
			return 0;
		}

		// Checks the environment variable `MINIJAVA_STACK_LIMIT` and, if it
		// is set, adjust the resource limt accordingly.  This function handles
		// erros by printing a warning to `err` and otherwise ignoring them,
		// letting the stack limit as it is.
		void try_adjust_stack_limit(std::ostream& err)
		{
			if (const auto limit = get_stack_limit(err)) {
				try {
					set_max_stack_size_limit(limit);
				} catch (const std::system_error& e) {
					// TODO: Once we have a logging facility, we should use it
					// here instead of printing directly.
					err << "warning: " << e.what() << std::endl;
				}
			}
		}

	}  // namespace /* anonymous */


	void real_main(const std::vector<const char *>& args,
	               std::istream& thestdin,
	               std::ostream& thestdout,
	               std::ostream& thestderr)
	{
		auto setup = program_setup{};
		if (!parse_cmd_options(args, thestdout, setup)) {
			finalize_stream(thestdout);
			return;
		}
		try_adjust_stack_limit(thestderr);
		std::ifstream istr{};
		std::ofstream ostr{};
		const auto usestdin = (setup.input == "-");
		const auto usestdout = (setup.output == "-");
		if (!usestdin) {
			istr.open(setup.input, std::ios_base::binary | std::ios_base::in);
			if (!istr) {
				const auto ec = std::error_code{errno, std::system_category()};
				throw std::system_error{ec, "Cannot open input file: " + setup.input};
			}
		}
		if (!usestdout) {
			ostr.open(setup.output, std::ios_base::binary | std::ios_base::out);
			if (!ostr) {
				const auto ec = std::error_code{errno, std::system_category()};
				throw std::system_error{ec, "Cannot open output file: " + setup.output};
			}
		}
		run_compiler_with_streams(
			(usestdin ? thestdin :  istr), (usestdout ? thestdout :  ostr),
			setup.stage
		);
	}

}  // namespace minijava
