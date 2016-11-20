#include "cli.hpp"

#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <sstream>  // Oh, dear!
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include "exceptions.hpp"
#include "global.hpp"
#include "io/file_data.hpp"
#include "io/file_output.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token_iterator.hpp"
#include "parser/parser.hpp"
#include "parser/pretty_printer.hpp"
#include "semantic/constant.hpp"
#include "semantic/ref_type_analysis.hpp"
#include "semantic/symbol_def.hpp"
#include "semantic/thou_shalt_return.hpp"
#include "semantic/unique_entry_point.hpp"
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


		// Prints the version text to `out`.
		void print_version(file_output& out)
		{
			out.print("%s %s\n", MINIJAVA_PROJECT_NAME, MINIJAVA_PROJECT_VERSION);
			out.write(
				"Copyright (C) 2016 T. Kahlert, P.J. Serrer, M. Baumann and M. Klammler\n"
				"This is free software; see the source for copying conditions.  There is NO\n"
				"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
			);
		}


		// Prints the help text to `out`.  `groups` is a list of pointers to
		// the option groups to include in the help text.
		void print_help(file_output& out, std::initializer_list<po::options_description*> groups)
		{
			out.print("usage: %s [OPTIONS] FILE\n\n", MINIJAVA_PROJECT_NAME);
			auto oss = std::ostringstream{};
			for (auto&& gp : groups) {
				oss << *gp << '\n';
			}
			out.write(oss.str());
			out.write(
				"Anywhere a file name is expected, '-' can be used to refer to the standard\n"
				"input or output stream respectively\n"
			);
		}


		// Determines the compilation stage at which to intercept from the
		// parsed command-line options in `varmap`.  The behavior is undefined
		// if more than one flag is set in the map.
		compilation_stage get_interception_stage(const po::variables_map& varmap)
		{
			// If you know a better solution for this, please share your
			// knowledge with us: https://stackoverflow.com/q/40305153/
			if (varmap.count("echo")) {
				return compilation_stage::input;
			}
			if (varmap.count("lextest")) {
				return compilation_stage::lexer;
			}
			if (varmap.count("parsetest")) {
				return compilation_stage::parser;
			}
			if (varmap.count("print-ast")) {
				return compilation_stage::print_ast;
			}
			if (varmap.count("check")) {
				return compilation_stage::semantic;
			}
			return compilation_stage{};
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
		                       file_output& out, program_setup& setup)
		{
			auto generic = po::options_description{"Generic Options"};
			generic.add_options()
				("help", "show help text and exit")
				("version", "show version text and exit");
			auto interception = po::options_description{"Intercepting the Compilation at Specific Stages"};
			interception.add_options()
				("echo", "stop after reading input and output it verbatim")
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
			if (varmap.count("help")) {
				print_help(out, {&generic, &interception, &other});
				return false;
			}
			if (varmap.count("version")) {
				print_version(out);
				return false;
			}
			po::notify(varmap);
			check_mutex_option_group(interception, varmap);
			setup.stage = get_interception_stage(varmap);
			return true;
		}


		// Prints the token `tok` to `out` in the formet required for
		// `--lextest`.  This function could be optimized to avoid the string
		// formatting but the fun for tweaking this stage is probably over now.
		void print_token(file_output& out, const token& tok)
		{
			if (tok.has_lexval()) {
				out.print("%s %s\n", name(tok.type()), tok.lexval().c_str());
			} else {
				out.print("%s\n", name(tok.type()));
			}
		}

		// Prints the AST `ast` to `out` in the formet required for
		// `--print-ast`.  This function should be optimized to avoid the
		// string-stream.
		void print_ast(file_output& out, const ast::node& ast)
		{
			auto oss = std::ostringstream{};
			auto pp = ast::pretty_printer{oss};
			ast.accept(pp);
			out.write(oss.str());
		}

		// Runs the compiler reading input from `istr`, writing output to
		// `ostr` and optionally intercepting compilation at `stage`.
		void run_compiler(file_data& in, file_output& out,
		                  const compilation_stage stage)
		{
			if (stage == compilation_stage::input) {
				out.write(in.data(), in.size());
				return;
			}
			auto pool = symbol_pool<>{};  // TODO: Use an appropriate allocator
			auto lex = make_lexer(std::begin(in), std::end(in), pool, pool);
			const auto tokfirst = token_begin(lex);
			const auto toklast = token_end(lex);
			if (stage == compilation_stage::lexer) {
				std::for_each(tokfirst, toklast, [&out](auto&& t){ print_token(out, t); });
				return;
			}
			auto ast = parse_program(tokfirst, toklast);
			if (stage == compilation_stage::parser) {
				return;
			}
			if (stage == compilation_stage::print_ast) {
				print_ast(out, *ast);
				return;
			}
			if (stage == compilation_stage::semantic) {
				auto defa = semantic::def_annotations{};
				auto typs = semantic::extract_typesystem(*ast, defa, pool);
				auto syst = semantic::builtins::register_system(typs, pool);
				semantic::analyse_program(*ast, {{pool.normalize("System"), syst}}, typs, defa);
				const auto handler = [](const ast::node& n){
					// TODO: This is very inappropriate but this is throw-away code.
					std::cerr << "warning: Expression has undefined result: "
					          << "line = " << n.line() << ", "
					          << "column = " << n.column() << "\n";
				};
				extract_constants(*ast, handler);
				check_return_paths(*ast);
				check_unique_entry_point(*ast);
				return;
			}
			// If we get until here, we have a problem...
			throw not_implemented_error{"The rest of the compiler has yet to be written"};
		}


		// Checks the environment variable `MINIJAVA_STACK_LIMIT` and
		// `return`s its value.  If the variable is set in the environment and
		// has a valid value, its value is `return`ed.  Otherwise, 0 (which is
		// not a valid value) is `return`ed.  If it is set to an invalid value,
		// a warning is printed to `err`.
		std::ptrdiff_t get_stack_limit(std::FILE* err)
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
			std::fprintf(
				err, "%s: warning: %s: %s: %s\n",
				MINIJAVA_PROJECT_NAME, MINIJAVA_ENVVAR_STACK_LIMIT,
			    "not a valid stack size in bytes", envval
			);
			return 0;
		}

		// Checks the environment variable `MINIJAVA_STACK_LIMIT` and, if it
		// is set, adjust the resource limt accordingly.  This function handles
		// erros by printing a warning to `err` and otherwise ignoring them,
		// letting the stack limit as it is.
		void try_adjust_stack_limit(std::FILE* err)
		{
			if (const auto limit = get_stack_limit(err)) {
				try {
					set_max_stack_size_limit(limit);
				} catch (const std::system_error& e) {
					// TODO: Once we have a logging facility, we should use it
					// here instead of printing directly.
					std::fprintf(
						err, "%s: warning: %s\n",
						MINIJAVA_PROJECT_NAME, e.what()
					);
				}
			}
		}

	}  // namespace /* anonymous */


	void real_main(const std::vector<const char*>& args,
	               std::FILE* thestdin,
	               std::FILE* thestdout,
	               std::FILE* thestderr)
	{
		auto setup = program_setup{};
		{
			auto out = file_output{thestdout};
			if (!parse_cmd_options(args, out, setup)) {
				out.flush();
				return;
			}
		}
		try_adjust_stack_limit(thestderr);
		auto in = (setup.input == "-")
			? file_data{thestdin, "stdin"}
			: file_data{setup.input};
		auto out = (setup.output == "-")
			? file_output{thestdout, "stdout"}
			: file_output{setup.output};
		run_compiler(in, out, setup.stage);
		out.finalize();
	}

}  // namespace minijava
