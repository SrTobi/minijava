#include "semantic/dont_use_main_args.hpp"

#include <string>
#include <iosfwd>
#include <sstream>

#include "parser/for_each_node.hpp"
#include "semantic/semantic_error.hpp"

namespace minijava
{
	namespace /* anonymous */
	{
		struct dont_use_main_args_visitor final : for_each_node
		{

			using for_each_node::visit;

			void visit(const ast::variable_access& node) override
			{
				if (_in_main && node.name() == _args_name && !node.target()) {
					std::ostringstream oss;
					oss << "Usage of " << _args_name << " is forbidden";
					throw semantic_error{oss.str()};
				}
			}

			void visit(const ast::main_method& node) override
			{
				_in_main = true;
				_args_name = node.argname();
				node.body().accept(*this);
				_in_main = false;
			}

		private:
			bool _in_main;
			symbol _args_name;
		};
	}

	void check_args_usage(const ast::program& ast)
	{
		auto visitor = dont_use_main_args_visitor{};
		ast.accept(visitor);
	}
}
