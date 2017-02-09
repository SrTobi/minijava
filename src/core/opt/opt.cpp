#include "opt/opt.hpp"
#include "opt/conditional.hpp"
#include "opt/control_flow.hpp"
#include "opt/folding.hpp"
#include "opt/inline.hpp"
#include "opt/unroll.hpp"
#include "opt/unused_params.hpp"
#include "opt/unused_method.hpp"
#include "opt/load_store.hpp"
#include "opt/tailrec.hpp"
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
		// should run at first, because after inlining, we couldn't remove them
		register_optimization(std::make_unique<opt::unused_params>());
		// important to get rid of unnecessary methods - no need to optimize them or even create code
		// they might be created from unused_params opt
		register_optimization(std::make_unique<opt::unused_method>());
		register_optimization(std::make_unique<opt::folding>());
		register_optimization(std::make_unique<opt::load_store>());
		register_optimization(std::make_unique<opt::conditional>());
		register_optimization(std::make_unique<opt::unroll>());
		register_optimization(std::make_unique<opt::control_flow>());
		register_optimization(std::make_unique<opt::tailrec>());
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

	std::vector<std::pair<firm::ir_node*, int>> opt::get_block_succ(firm::ir_node *node)
	{
		auto vec = std::vector<std::pair<firm::ir_node*, int>>();
		for (auto out_block = firm::get_irn_out_edge_first_kind(node, firm::EDGE_KIND_BLOCK); out_block; out_block = firm::get_irn_out_edge_next(node, out_block, firm::EDGE_KIND_BLOCK)) {
			vec.push_back(std::make_pair(firm::get_edge_src_irn(out_block), firm::get_edge_src_pos(out_block)));
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
		auto inputs = std::vector<firm::ir_node*>();
		for (int i = 0; i < arity; i++) {
			inputs.push_back((firm::ir_node*)firm::get_irn_link(firm::get_irn_n(n, i)));
		}
		nn = firm::new_ir_node(
				firm::get_irn_dbg_info(n),
				irg,
				nullptr,
				firm::get_irn_op(n),
				firm::get_irn_mode(n),
				arity,
				inputs.data()
		);
		firm::set_irn_link(n, nn);
		firm::copy_node_attr(irg, n, nn);
		return nn;
	}

	bool opt::is_nop(firm::ir_node* node)
	{
		switch (firm::get_irn_opcode(node)) {
			case firm::iro_Anchor:
			case firm::iro_Bad:
			case firm::iro_Confirm:
			case firm::iro_Deleted:
			case firm::iro_Dummy:
			case firm::iro_End:
			case firm::iro_Id:
			case firm::iro_NoMem:
			case firm::iro_Pin:
			case firm::iro_Proj:
			case firm::iro_Start:
			case firm::iro_Sync:
			case firm::iro_Tuple:
			case firm::iro_Unknown:
				return true;
			case firm::iro_Phi:
				return firm::get_irn_mode(node) == firm::mode_M;
			default:
				return false;
		}
	}

	bool opt::is_tarval_numeric(firm::ir_tarval* val)
	{
		return val && firm::get_mode_arithmetic(firm::get_tarval_mode(val)) == firm::irma_twos_complement;
	}

	bool opt::is_tarval_with_num(firm::ir_tarval* val, long num)
	{
		return is_tarval_numeric(val) && firm::get_tarval_long(val) == num;
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
