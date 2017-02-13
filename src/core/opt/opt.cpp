#include "opt/opt.hpp"

#include <algorithm>
#include <cassert>
#include <queue>
#include <unordered_map>

#include "irg/irg.hpp"
#include "opt/conditional.hpp"
#include "opt/control_flow.hpp"
#include "opt/folding.hpp"
#include "opt/gc.hpp"
#include "opt/inline.hpp"
#include "opt/load_store.hpp"
#include "opt/lowering.hpp"
#include "opt/ssa_helper.hpp"
#include "opt/unroll.hpp"
#include "opt/unused_params.hpp"
#include "opt/unused_method.hpp"
#include "opt/tailrec.hpp"

namespace minijava
{
	namespace {

		std::vector<std::unique_ptr<minijava::opt::optimization>> optimizations;

		template<typename T>
		struct opt_constr_impl
		{
			std::unique_ptr<opt::optimization> operator ()()
			{
				return std::make_unique<T>();
			}
		};

		using opt_constructor = std::function<std::unique_ptr<opt::optimization>()>;

		std::vector<std::pair<std::string, opt_constructor>> optConstructors = {
			// should run at first, because after inlining, we couldn't remove them
			//{ "unused_params", opt_constr_impl<opt::unused_params>{}},
			// important to get rid of unnecessary methods - no need to optimize them or even create code
			// they might be created from unused_params opt
			{ "unused_method", opt_constr_impl<opt::unused_method>{}},
			{ "folding", opt_constr_impl<opt::folding>{}},
			{ "load_store", opt_constr_impl<opt::load_store>{}},
			{ "conditional", opt_constr_impl<opt::conditional>{}},
			{ "unroll", opt_constr_impl<opt::unroll>{}},
			{ "control_flow", opt_constr_impl<opt::control_flow>{}},
			{ "tailrec", opt_constr_impl<opt::tailrec>{}},
			{ "inliner", opt_constr_impl<opt::inliner>{}},
			{ "gc", opt_constr_impl<opt::gc>{}},
		};

		const std::unordered_map<std::string, opt_constructor>& get_opt_constr_mapping()
		{
			static const std::unordered_map<std::string, opt_constructor> mapping {std::begin(optConstructors), std::end(optConstructors)};
			return mapping;
		}

		std::vector<std::string> make_opt_names()
		{
			std::vector<std::string> result{};
			for(const auto& p : optConstructors)
			{
				result.push_back(p.first);
			}
			return result;
		}
	}

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
		auto helper = opt::ssa_helper();
		helper.optimize(ir);
		opt::lower();
	}

	void register_optimization(std::unique_ptr<minijava::opt::optimization> opt)
	{
		optimizations.push_back(std::move(opt));
	}

	void register_all_optimizations()
	{
		// loop over all optimizations and add them
		for(auto& opt_p : optConstructors)
		{
			register_optimization(opt_p.second());
		}
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

	void register_optimization(const std::string& opt)
	{
		auto& mapping = get_opt_constr_mapping();
		auto it = mapping.find(opt);

		if(it == mapping.end())
		{
			throw std::runtime_error("no known optimization '" + opt + "'");
		}
		register_optimization(it->second());
	}

	const std::vector<std::string>& get_optimization_names()
	{
		static const auto names = make_opt_names();
		return names;
	}

}  // namespace minijava
