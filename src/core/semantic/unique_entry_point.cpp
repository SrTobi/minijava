#include "semantic/unique_entry_point.hpp"

#include "parser/for_each_node.hpp"
#include "semantic/semantic_error.hpp"

namespace minijava
{
	namespace /* anonymous */
	{
		struct unique_entry_point_visitor final : for_each_node
		{

			bool found{false};

			using for_each_node::visit;

			void visit(const ast::main_method&) override
			{
				if (found) {
					throw semantic_error{"Program has multiple entry points"};
				}
				found = true;
			}

		};
	}

	void check_unique_entry_point(const ast::program& ast)
	{
		auto visitor = unique_entry_point_visitor{};
		ast.accept(visitor);
	}
}
