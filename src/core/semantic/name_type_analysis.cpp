#include "semantic/name_type_analysis.hpp"

namespace minijava
{
	namespace sem
	{
		void perform_shallow_type_analysis(const ast::program& ast,
										   type_annotations& types)
		{
			// FIXME
			(void)ast;
			(void)types;
			throw nullptr;
		}

		void perform_full_name_type_analysis(const ast::program& ast,
											 const globals_map& globals,
											 type_annotations& types,
											 locals& locals,
											 var_declarations& var_decls,
											 method_declarations& method_decls)
		{
			perform_shallow_type_analysis(ast, types);
			// FIXME
			(void)globals;
			(void)locals;
			(void)var_decls;
			(void)method_decls;
			throw nullptr;
		}
	}
}
