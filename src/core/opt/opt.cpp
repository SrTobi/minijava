#include "opt/opt.hpp"
#include "opt/folding.hpp"
#include "opt/inline.hpp"
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
				changed = opt->optimize(ir) || changed;
			}
		} while (changed && count++ < max_count);
	}

	void register_optimization(std::unique_ptr<minijava::opt::optimization> opt)
	{
		optimizations.push_back(std::move(opt));
	}

	void register_all_optimizations()
	{
		register_optimization(std::make_unique<opt::folding>());
		register_optimization(std::make_unique<opt::inliner>());
	}

	std::vector<std::pair<firm::ir_node*, int>> opt::get_out_edges_safe(firm::ir_node *node)
	{
		auto vec = std::vector<std::pair<firm::ir_node*, int>>();
		for (auto out_edge = firm::get_irn_out_edge_first(node); out_edge; out_edge = firm::get_irn_out_edge_next(node, out_edge, firm::EDGE_KIND_NORMAL)) {
			vec.push_back(std::make_pair(firm::get_edge_src_irn(out_edge), firm::get_edge_src_pos(out_edge)));
		}
		return vec;
	}

	firm::ir_node* opt::copy_irn_to_irg(firm::ir_node *n, firm::ir_graph *irg)
	{
		firm::ir_node* nn = nullptr;
		auto old_irg = firm::get_irn_irg(n);
		switch (firm::get_irn_opcode(n)) {
			case firm::iro_NoMem:
				nn = firm::get_irg_no_mem(irg);
				break;
			case firm::iro_Block:
				if (n == firm::get_irg_start_block(old_irg))
					nn = firm::get_irg_start_block(irg);
				else if (n == firm::get_irg_end_block(old_irg))
					nn = firm::get_irg_end_block(irg);
				break;
			case firm::iro_Start:
				nn = firm::get_irg_start(irg);
				break;
			case firm::iro_End:
				nn = firm::get_irg_end(irg);
				break;
			case firm::iro_Proj:
				if (n == firm::get_irg_initial_mem(old_irg))
					nn = firm::get_irg_initial_mem(irg);
				else if (n == firm::get_irg_args(old_irg))
					nn = firm::get_irg_args(irg);
				break;
		}

		if (nn) {
			// link old with new node
			firm::set_irn_link(n, nn);
			return nn;
		}

		auto arity = firm::get_irn_arity(n);
		firm::ir_node** inputs = new firm::ir_node*[firm::get_irn_arity(n)];
		for (int i = 0; i < arity; i++) {
			inputs[i] = (firm::ir_node*)firm::get_irn_link(firm::get_irn_n(n, i));
		}
		nn = firm::new_ir_node(
				firm::get_irn_dbg_info(n),
				irg,
				nullptr,
				firm::get_irn_op(n),
				firm::get_irn_mode(n),
				arity,
				inputs
		);
		firm::set_irn_link(n, nn);
		firm::copy_node_attr(irg, n, nn);
		return nn;
	}

	void copy_nodes(firm::ir_node* node, void* env)
	{
		opt::copy_irn_to_irg(node, (firm::ir_graph*)env);
	}

	void set_preds(firm::ir_node* node, void* env)
	{
		auto new_irg = (firm::ir_graph*)env;
		auto nn = (firm::ir_node*)firm::get_irn_link(node);

		if (firm::is_Block(node)) {
			auto irg = firm::get_irn_irg(node);
			auto end_block = firm::get_irg_end_block(irg);
			for (int i = firm::get_Block_n_cfgpreds(node); i-- > 0;) {
				auto pred = firm::get_Block_cfgpred(node, i);
				if (end_block == node) {
					firm::add_immBlock_pred(firm::get_irg_end_block(new_irg), (firm::ir_node*)firm::get_irn_link(pred));
				} else {
					firm::set_Block_cfgpred(nn, i, (firm::ir_node*)firm::get_irn_link(pred));
				}
			}
		} else {
			firm::set_nodes_block(nn, (firm::ir_node*)firm::get_irn_link(firm::get_nodes_block(node)));
			if (firm::is_End(node)) {
				for (int i = 0, nodes = firm::get_End_n_keepalives(node); i < nodes; ++i) {
					firm::add_End_keepalive(nn, (firm::ir_node*)firm::get_irn_link(firm::get_End_keepalive(node, i)));
				}
			} else {
				// #foreach_irn_in_r macro from irnode_t.h
				for (bool pred__b = true; pred__b;) {
					for (firm::ir_node* pred__irn = node; pred__b; pred__b = false) {
						for (int idx = firm::get_irn_arity(pred__irn); pred__b && idx-- != 0;) {
							for (firm::ir_node* pred = (pred__b = false, firm::get_irn_n(pred__irn, idx)); !pred__b; pred__b = true) {
								set_irn_n(nn, idx, (firm::ir_node*)firm::get_irn_link(pred));
							}
						}
					}
				}
			}
		}
	}

	void opt::clone_irg(firm::ir_graph* from, firm::ir_graph* to)
	{
		firm::irg_walk_graph(from, copy_nodes, set_preds, to);
		firm::irg_finalize_cons(to);
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
