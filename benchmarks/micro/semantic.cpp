#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <vector>

#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "parser/ast_misc.hpp"
#include "semantic/semantic.hpp"
#include "symbol/symbol_pool.hpp"

#include "testaux/astgen.hpp"
#include "testaux/benchmark.hpp"


namespace /* anonymous */
{

	using pool_type = minijava::symbol_pool<>;


	void benchmark(const minijava::ast::program& ast,
	               pool_type& pool, minijava::ast_factory& factory)
	{
		testaux::clobber_memory(&ast);
		const auto seminfo = minijava::check_program(ast, pool, factory);
	}


	void real_main(int argc, char** argv)
	{
		const auto t0 = testaux::clock_type::now();
		auto setup = testaux::benchmark_setup{
			"semantic",
			"Benchmark for performance of pure semantic analysis without AST construction."
		};
		setup.add_cmd_arg("recursion-depth", "recursion depth for deriving the input");
		setup.add_cmd_flag("print", "print the sample data to standard error output");
		if (!setup.process(argc, argv)) {
			return;
		}
		const auto depth = setup.get_cmd_arg("recursion-depth");
		auto engine = testaux::get_random_engine();
		auto pool = pool_type{};
		auto factory = minijava::ast_factory{};
		const auto ast = testaux::generate_semantic_ast(engine, pool, factory, depth);
		const auto size = factory.id();
		if (setup.get_cmd_flag("print")) {
			std::clog << "/* Randomly generated MiniJava program.  */\n"
					  << "/* Number of AST nodes:     " << std::setw(12) << size  << " */\n"
					  << "/* Maximum recursion depth: " << std::setw(12) << depth << " */\n"
					  << "\n"
					  << *ast << std::flush;
		}
		auto constr = setup.get_constraints();
		if (constr.timeout.count() > 0) {
			constr.timeout -= testaux::duration_type{testaux::clock_type::now() - t0};
		}
		const auto absres = testaux::run_benchmark(constr, benchmark, *ast, pool, factory);
		const auto relres = testaux::result{absres.mean / size, absres.stdev / size, absres.n};
		testaux::print_result(relres);
	}

}  // namespace /* anonymous */


int main(int argc, char * * argv)
{
	try {
		real_main(argc, argv);
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "semantic: error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
