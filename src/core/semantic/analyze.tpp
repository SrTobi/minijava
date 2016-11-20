#ifndef MINIJAVA_INCLUDED_FROM_SEMANTIC_ANALYZE_HPP
#error "Never `#include <semantic/analyze.tpp>` directly; `#include <semantic/analyze.hpp>` instead."
#endif

#include "semantic/builtins.hpp"
#include "semantic/constant.hpp"
#include "semantic/ref_type_analysis.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/symbol_def.hpp"
#include "semantic/thou_shalt_return.hpp"
#include "semantic/unique_entry_point.hpp"

namespace minijava
{
	template<typename AllocT>
	void analyze_ast(const ast::program& ast, symbol_pool<AllocT>& pool)
	{
		auto defa = semantic::def_annotations{};
		auto typs = semantic::extract_typesystem(ast, defa, pool);
		auto syst = semantic::builtins::register_system(typs, pool);
		auto annotations = semantic::analyse_program(ast, {{pool.normalize("System"), syst}}, typs, defa);
		const auto handler = [](const ast::node& /* n */){
			// FIXME: add source code location to exception object
			// FIXME: actually throw exception (binary_arithmetic.mj is supposed to be valid ATM)
			// throw semantic_error{"Result of expression is undefined"};
		};
		auto constants = extract_constants(ast, handler);
		check_return_paths(ast);
		check_unique_entry_point(ast);
		// FIXME: return everything we want to keep
		(void) annotations;
		(void) constants;
	}
}
