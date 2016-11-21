#include "semantic/dont_use_main_args.hpp"

#include <string>
#include <iosfwd>
#include <sstream>

#include "parser/for_each_node.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/symbol_def.hpp"
#include "semantic/ref_type_analysis.hpp"

namespace minijava
{
	using namespace semantic;

	namespace /* anonymous */
	{
		struct dont_use_main_args_visitor final : for_each_node
		{

			using for_each_node::visit;

			dont_use_main_args_visitor(const def_annotations& def_a, const ref_annotation& ref_a)
				: _def_a(def_a)
				, _ref_a(ref_a)
			{
			}

			void visit(const ast::variable_access& node) override
			{
				if (&_ref_a[node] == _arg_def) {
					std::ostringstream oss;
					oss << "Usage of " << node.name() << " is forbidden";
					throw semantic_error{oss.str()};
				}
			}

			void visit(const ast::main_method& node) override
			{
				_arg_def = _def_a[node].parameters().front();
				do_visit(node.body());
			}

		private:
			const var_def* _arg_def = nullptr;
			const def_annotations& _def_a;
			const ref_annotation& _ref_a;
		};
	}

	void check_args_usage(const ast::program& ast, const def_annotations& def_a, const ref_annotation& ref_a)
	{
		auto visitor = dont_use_main_args_visitor{ def_a, ref_a};
		ast.accept(visitor);
	}
}
