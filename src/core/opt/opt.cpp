#include "opt/opt.hpp"
#include "opt/algebraic_simplifier.hpp"
#include "opt/load_store.hpp"
#include <queue>

namespace minijava
{

	std::vector<std::unique_ptr<minijava::opt::optimization>> optimizations;

	void optimize(firm_ir& ir)
	{
		const auto guard = make_irp_guard(*ir->second, ir->first);
		bool changed;
		unsigned int count = 0;
		unsigned int max_count = 99;
		do
		{
			changed = false;
			for (auto& opt : optimizations) {
				changed |= opt->optimize(ir);
			}
		} while (changed && count++ < max_count);
	}

	void register_optimization(std::unique_ptr<minijava::opt::optimization> opt)
	{
		optimizations.push_back(std::move(opt));
	}

	void register_all_optimizations()
	{
		register_optimization(std::make_unique<opt::algebraic_simplifier>());
		register_optimization(std::make_unique<opt::load_store>());
	}

	std::vector<std::pair<firm::ir_node*, int>> opt::get_out_edges_safe(firm::ir_node *node)
	{
		auto vec = std::vector<std::pair<firm::ir_node*, int>>();
		for (auto out_edge = firm::get_irn_out_edge_first(node); out_edge; out_edge = firm::get_irn_out_edge_next(node, out_edge, firm::EDGE_KIND_NORMAL)) {
			vec.push_back(std::make_pair(firm::get_edge_src_irn(out_edge), firm::get_edge_src_pos(out_edge)));
		}
		return vec;
	}

	// worklist stuff

	bool opt::worklist_optimization::optimize(firm_ir& /*ir*/)
	{
		_changed = false;
		auto n = firm::get_irp_n_irgs();
		for (size_t i = 0; i < n; i++) {
			_irg = firm::get_irp_irg(i);
			firm::ir_reserve_resources(_irg, firm::IR_RESOURCE_IRN_LINK);
			firm::edges_activate(_irg);
			// run worklist
			auto worklist = opt::worklist(_irg);
			worklist.run(this);
			// cleanup
			firm::irg_walk_topological(_irg, [](firm::ir_node* node, void* env) {
				((opt::worklist_optimization*)env)->cleanup(node);
			}, this);
			firm::edges_deactivate(_irg);
			firm::ir_free_resources(_irg, firm::IR_RESOURCE_IRN_LINK);
			firm::remove_unreachable_code(_irg);
			firm::remove_bads(_irg);
		}
		return _changed;
	}

	void opt::worklist_optimization::cleanup(firm::ir_node* /*node*/)
	{ }

	opt::worklist::worklist(firm::ir_graph* irg) : _irg{irg}
	{ }

	void opt::worklist::run(worklist_optimization *opt) {
		auto queue = worklist_queue();
		// collect nodes of current irg topological in worklist_queue
		firm::irg_walk_topological(_irg, [](firm::ir_node* node, void* env) {
			((opt::worklist_queue*)env)->push(node);
		}, &queue);
		// while there is something to do..
		while (!queue.empty()) {
			auto node = queue.front();
			queue.pop();
			if (opt->handle(node)) {
				auto outs = get_out_edges_safe(node);
				for (std::pair<firm::ir_node*, int> out : outs) {
					queue.push(out.first);
				}
			}
		}
	}

}  // namespace minijava
