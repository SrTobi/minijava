#include "cli.hpp"

#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <tuple>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include "asm/asm.hpp"
#include "asm/firm_backend.hpp"
#include "exceptions.hpp"
#include "global.hpp"
#include "io/file_cleanup.hpp"
#include "io/file_data.hpp"
#include "io/file_output.hpp"
#include "irg/irg.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token_iterator.hpp"
#include "opt/opt.hpp"
#include "parser/ast_misc.hpp"
#include "parser/parser.hpp"
#include "runtime/host_cc.hpp"
#include "semantic/semantic.hpp"
#include "source_error.hpp"
#include "symbol/symbol_pool.hpp"
#include "system/logger.hpp"
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
			lexer,
			parser,
			print_ast,
			semantic,
			dump_ir,
			dump_ir_opt,
			compile_firm,
		};


		// Parsed command-line options.
		struct program_setup
		{
			// Stage at which the compilation should be intercepted.
			compilation_stage stage{};

			// Name of the input file (may be `-` to read from stdin).
			std::string input{};

			// Name of the output file (may be `-` to write to stdout).
			std::string output{};

			// Name of the C compiler executable (for linking the runtime)
			std::string cc{};

			// Prevent output to the log?
			bool quiet = false;

			// Ordered list of optimizations
			std::vector<std::string> optimizations{};
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
			out.print("%s %s (%s)\n", MINIJAVA_PROJECT_NAME, MINIJAVA_PROJECT_VERSION, "https://minij.slixe.de/");
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
		// Prints a list of available optimizations to `out`.
		void print_opts_list(file_output& out)
		{
			const auto& opts = get_optimization_names();

			int i = 1;
			for(const auto& name : opts)
			{
				out.print("%i. %s\n", i, name.c_str());
				++i;
			}
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
			if (varmap.count("dump-ir")) {
				return compilation_stage::dump_ir;
			}
			if (varmap.count("dump-ir-opt")) {
				return compilation_stage::dump_ir_opt;
			}
			if (varmap.count("compile-firm")) {
				return compilation_stage::compile_firm;
			}
			return compilation_stage{};
		}

		// Sorts a list of optimizations according to the available optimizations.
		// Duplications will be ereased
		void sort_optimizations(std::vector<std::string>& opts)
		{
			const auto& available_opts = get_optimization_names();

			std::vector<std::string> ordered{};
			for(const auto& opt : available_opts)
			{
				if(std::count(opts.begin(), opts.end(), opt)) {
					ordered.push_back(opt);
				}
			}
			opts = std::move(ordered);
		}

		// Checks if a list contains names that are not optimizations.
		// Reports invalid optimization names
		void check_optimizations(const std::vector<std::string>& opts, file_output& out)
		{
			bool error = false;
			const auto& available_opts = get_optimization_names();
			for (const auto& opt : opts) {
				if(!std::count(available_opts.begin(), available_opts.end(), opt)) {
					out.print("unknown optimization: %s!\n", opt.c_str());
					error = true;
				}
			}
			if(error) {
				out.print("Use --opts-list for a list of available optimizations\n");
				throw std::runtime_error("Unknown optimizations specified");
			}
		}

		std::vector<std::string> get_optimizations(const po::variables_map& varmap, file_output& out)
		{
			const auto& default_opts = get_optimization_names();

			if (varmap.count("O")) {
				switch(varmap["O"].as<unsigned int>())
				{
				case 0:
					return {};
				case 1:
					return {"folding"};
				default:
				case 2:
					return default_opts;
				};
			} else if (varmap.count("opts")) {
				std::vector<std::string> opts;
				boost::split(opts, varmap["opts"].as<std::string>(), boost::is_any_of(","));
				check_optimizations(opts, out);
				sort_optimizations(opts);
				return opts;
			} else if (varmap.count("opts-ordered")) {
				std::vector<std::string> opts;
				boost::split(opts, varmap["opts-ordered"].as<std::string>(), boost::is_any_of(","));
				check_optimizations(opts, out);
				return opts;
			}

			return default_opts;
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
				("version", "show version text and exit")
				("quiet", "prevent log output");
			auto interception = po::options_description{"Intercepting the Compilation at Specific Stages"};
			interception.add_options()
				("echo", "stop after reading input and output it verbatim")
				("lextest", "stop after lexical analysis and output a token sequence")
				("parsetest", "stop after parsing and reporting any syntax errors")
				("print-ast", "stop after parsing and print the parsed ast")
				("check", "stop after semantic analysis and report semantic errors")
				("dump-ir", "stop after IR creation and dump the intermediate representation into the current directory")
				("dump-ir-opt", "same as --dump-ir but only stop after optimizing / lowering")
				("compile-firm", "stop after IR creation and compile the input using the firm backend");
			auto other = po::options_description{"Other Options"};
			other.add_options()
				("cc", po::value<std::string>(&setup.cc)->default_value(get_default_c_compiler()), "C compiler to use for linking the runtime")
				("output", po::value<std::string>(&setup.output)->default_value("-"), "redirect output to file");
			auto inputfiles = po::options_description{"Input Files"};
			inputfiles.add_options()
				("input", po::value<std::string>(&setup.input)->default_value("-"), "");
			auto opts = po::options_description{"Optimization"};
			opts.add_options()
				("O,O", po::value<unsigned int>(), "Optimization level (-O0, -O1, -O3)")
				("opts-list", "list available optimizations")
				("opts", po::value<std::string>(), "turn on specific optimizations")
				("opts-ordered", po::value<std::string>(), "turn on specific optimizations in defined order");
			auto options = po::options_description{};
			options.add(generic).add(interception).add(other).add(inputfiles).add(opts);
			auto positional = po::positional_options_description{};
			positional.add("input", 1);
			auto varmap = po::variables_map{};
			const int argc = static_cast<int>(args.size());  // safe cast
			po::store(po::command_line_parser(argc, args.data())
			         .options(options).positional(positional).run(), varmap);
			if (varmap.count("help")) {
				print_help(out, {&generic, &interception, &opts, &other});
				return false;
			}
			if (varmap.count("version")) {
				print_version(out);
				return false;
			}

			if (varmap.count("quiet")) {
				setup.quiet = true;
			}

			if (varmap.count("opts-list")) {
				print_opts_list(out);
				return false;
			}
			po::notify(varmap);
			check_mutex_option_group(interception, varmap);
			setup.stage = get_interception_stage(varmap);
			setup.optimizations = get_optimizations(varmap, out);
			return true;
		}


		// Prints the token `tok` to `out` in the format required for
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

		void run_compiler_stages(file_data& in, file_output& out,
		                         const compilation_stage stage, const std::string& cc, symbol_pool<>& pool,
		                         const std::vector<std::string>& optimizations)
		{
			namespace fs = boost::filesystem;
			using namespace std::string_literals;

			auto lex = make_lexer(std::begin(in), std::end(in), pool, pool);
			const auto tokfirst = token_begin(lex);
			const auto toklast = token_end(lex);
			if (stage == compilation_stage::lexer) {
				std::for_each(tokfirst, toklast, [&out](auto&& t){ print_token(out, t); });
				return;
			}
			auto factory = ast_factory{};
			auto ast = parse_program(tokfirst, toklast, factory);
			if (stage == compilation_stage::parser) {
				return;
			}
			if (stage == compilation_stage::print_ast) {
				out.write(to_text(*ast));
				return;
			}
			auto sem_info = check_program(*ast, pool, factory);
			if (stage == compilation_stage::semantic) {
				return;
			}
			auto firm = initialize_firm();
			auto ir = create_firm_ir(*firm, *ast, sem_info, in.filename());
			if (stage == compilation_stage::dump_ir) {
				dump_firm_ir(ir);  // TODO: allow setting directory
				return;
			}
			// optimize
			for(const auto& opt_name : optimizations) {
				register_optimization(opt_name);
			}
			optimize(ir);
			if (stage == compilation_stage::dump_ir_opt) {
				dump_firm_ir(ir);  // TODO: allow setting directory
				return;
			}
			// From now on, output defaults to 'a.out'/'a.exe', not to stdout.
			if (out.filename().empty()) {
				out = file_output{MINIJAVA_WINDOWS_ASSEMBLY ? "a.exe" : "a.out"};
			}
			const auto tempdir = fs::temp_directory_path();
			const auto asmname = fs::unique_path(tempdir / "%%%%%%%%%%%%.s").string();
			const file_cleanup asm_cleanup_guard{asmname};
			auto asmout = file_output{asmname};
			if (stage == compilation_stage::compile_firm) {
				emit_x64_assembly_firm(ir, asmout);
			} else {
				assert(stage == compilation_stage{});
				assemble(ir, asmout);
			}
			asmout.close();
			link_runtime(cc, out.filename(), asmname);
		}

		std::tuple<std::size_t, std::size_t, std::string>
			get_line_section_by_position(file_data& data, const position& pos,
			                             const std::size_t max_length, const std::size_t min_skip)
		{
			assert(pos.line() >= 1);
			std::size_t line = 1;
			auto cur = data.begin();
			while(line < pos.line()) {
				while(*cur != '\n') {
					++cur;
				}
				++cur; // skip \n
				++line;
			}
			const auto line_begin = cur;

			while(cur != data.end() && *cur != '\n') {
				++cur;
			}
			const auto line_end = cur;

			const std::size_t line_length = static_cast<std::size_t>(std::distance(line_begin, line_end));
			if (line_length <= max_length) {
				return std::make_tuple(0, 0, std::string(line_begin, line_end));
			} else {
				const std::size_t column_position_in_section = max_length / 2;
				const std::size_t skip_suggestion = pos.column() < column_position_in_section? 0 : pos.column() - column_position_in_section;
				const std::size_t skip = (skip_suggestion >= min_skip? skip_suggestion : 0);
				auto section_begin = line_begin + skip;
				auto section_end = std::min(line_end, section_begin + max_length);
				const std::size_t skip_after = static_cast<std::size_t>(section_end - section_begin);
				return std::make_tuple(skip, skip_after, std::string(section_begin,  section_end));
			}
		}

		void print_source_error(logger& log, const source_error& e, file_data& in, const char* stage_verb)
		{
			log.printf("Error while %s %s:\n", stage_verb, in.filename().c_str());
			const position pos = e.position();
			if(pos == position{0, 0})
			{
				log.printf("  At an unknown location: %s\n", e.what());
			} else {
				log.printf("  In line %zu at column %zu: %s\n", pos.line(), pos.column(), e.what());

				std::size_t skip_chars;
				std::size_t skip_after;
				std::string section;
				std::tie(skip_chars, skip_after, section) = get_line_section_by_position(in, pos, 100, 10);

				// adjust section to support tabs
				int indent = static_cast<int>(pos.column() - skip_chars);
				indent += static_cast<int>(std::count(section.begin(), section.end(), '\t'));  // 2 spaces per tab
				boost::replace_all(section, "\t", "  ");

				auto print_after_skip = [&] {
					if(skip_after > 0) {
						log.printf(" <%zu skipped>", skip_after);
					}
				};

				if(skip_chars > 0) {
					indent += static_cast<int>(std::to_string(skip_chars).size());
					log.printf("  <%zu skipped> %s", skip_chars, section.c_str());
					print_after_skip();
					log.printf("\n             %*s\n", indent, "^");
				}else{
					log.printf("  %s", section.c_str());
					print_after_skip();
					log.printf("\n  %*s\n", indent, "^");
				}
			}
		}

		// Runs the compiler reading input from `istr`, writing output to
		// `ostr` and optionally intercepting compilation at `stage`.
		void run_compiler(file_data& in, file_output& out, logger& log,
		                  const compilation_stage stage, const std::string& cc,
		                  const std::vector<std::string>& optimizations)
		{
			using namespace std::string_literals;
			if (stage == compilation_stage::input) {
				out.write(in.data(), in.size());
				return;
			}
			auto pool = symbol_pool<>{};  // TODO: Use an appropriate allocator

			try {
				run_compiler_stages(in, out, stage, cc, pool, optimizations);
			} catch(lexical_error& e) {
				print_source_error(log, e, in, "tokenizing");
				throw;
			} catch(syntax_error& e) {
				print_source_error(log, e, in, "parsing");
				throw;
			} catch(semantic_error& e) {
				print_source_error(log, e, in, "analysing the semantics of");
				throw;
			} catch(source_error& e) {
				print_source_error(log, e, in, "compiling");
				throw;
			}
		}


		// Checks the environment variable `MINIJAVA_STACK_LIMIT` and
		// `return`s its value.  If the variable is set in the environment and
		// has a valid value, its value is `return`ed.  Otherwise, 0 (which is
		// not a valid value) is `return`ed.  If it is set to an invalid value,
		// a warning is printed to `err`.
		std::ptrdiff_t get_stack_limit(logger& log)
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
			log.printf("%s: warning: %s: %s: %s\n",
				MINIJAVA_PROJECT_NAME, MINIJAVA_ENVVAR_STACK_LIMIT,
			    "not a valid stack size in bytes", envval
			);
			return 0;
		}

		// Checks the environment variable `MINIJAVA_STACK_LIMIT` and, if it
		// is set, adjust the resource limt accordingly.  This function handles
		// erros by printing a warning to `err` and otherwise ignoring them,
		// letting the stack limit as it is.
		void try_adjust_stack_limit(logger& log)
		{
			if (const auto limit = get_stack_limit(log)) {
				try {
					set_max_stack_size_limit(limit);
				} catch (const std::system_error& e) {
					// TODO: Once we have a logging facility, we should use it
					// here instead of printing directly.
					log.printf(
						"%s: warning: %s\n",
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

		logger log = setup.quiet? logger{} : logger{thestderr};

		try_adjust_stack_limit(log);
		auto in = (setup.input == "-")
			? file_data{thestdin}
			: file_data{setup.input};
		auto out = (setup.output == "-")
			? file_output{thestdout}
			: file_output{setup.output};
		run_compiler(in, out, log, setup.stage, setup.cc, setup.optimizations);
		out.finalize();
	}

}  // namespace minijava
