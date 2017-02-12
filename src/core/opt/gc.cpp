#include "opt/gc.hpp"

#include <deque>

using namespace minijava::opt;

namespace {

	void unlink_mem_from_node(firm::ir_node *node) {
		for (auto &out_edge : get_out_edges_safe(node)) {
			const auto out_child = out_edge.first;
			if (firm::get_irn_mode(out_child) == firm::mode_M) {
				for (auto &child_edge : get_out_edges_safe(out_child)) {
					firm::set_irn_n(child_edge.first, child_edge.second, firm::get_irn_n(node, 0));
				}
			}
		}
	}

	class gc_walker
	{
	public:
		gc_walker(firm::ir_node* node)
			: _call(node)
		{
			nodes.push_back(node);
		}

		bool has_only_stores()
		{
			// expect a proj to unwrap the return values
			for (auto& e1 : get_out_edges_safe(_call)) {
				const auto proj1 = e1.first;
				if (!firm::is_Proj(proj1))
					return false;

				const auto proj1_mode = firm::get_irn_mode(proj1);
				if (proj1_mode == firm::mode_M)
					continue;

				if (proj1_mode != firm::mode_T)
					return false;

				// expect a proj to access the return values
				for(auto& e2 : get_out_edges_safe(proj1)) {
					const auto proj2 = e2.first;
					if (!firm::is_Proj(proj2) || firm::get_irn_mode(proj2) != firm::mode_P)
						return false;

					// expect a member address calculation
					for(auto& e3 : get_out_edges_safe(proj2)) {
						const auto member = e3.first;
						if(!firm::is_Member(member))
							return false;

						// finally always expect e stores
						for(auto& e4 : get_out_edges_safe(member)) {
							const auto store = e4.first;
							if(!firm::is_Store(store))
								return false;

							nodes.push_back(store);
						}
					}
				}
			}
			return true;
		}

		void remove_object()
		{
			for(auto node : nodes) {
				unlink_mem_from_node(node);
			}
		}

	private:
		firm::ir_node* _call;
		std::deque<firm::ir_node*> nodes{};
	};

}

bool gc::handle(firm::ir_node* node)
{
	// search for a call to mj_runtime_new
	if (!firm::is_Call(node))
		return false;

	const auto call_entity = firm::get_Call_callee(node);
	std::string name = firm::get_entity_name(call_entity);
	if (name != "mj_runtime_new")
		return false;

	gc_walker walker(node);

	if(!walker.has_only_stores())
		return false;

	walker.remove_object();
	_changed = true;
	return false;
}
