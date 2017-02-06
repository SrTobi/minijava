#include <firm.hpp>
#include "opt/unroll.hpp"

#include <map>
#include <iostream>

using namespace minijava::opt;


namespace /* anonymous */
{

	static const int MAX_LOOP_BRANCHES = 20;
	static const int MAX_LOOP_SIZE = 200;
	static const size_t MAX_LOOP_ITERATIONS = 10;


	static firm::ir_graph *current_irg;

	struct copy_node
	{
		std::vector<firm::ir_node*> copies;
	};

	static std::map<firm::ir_node*, copy_node*> copy_node_map;

	struct loop_edge {
		loop_edge(firm::ir_node *node, firm::ir_node *pred, int pos) : node{node}, pred{pred}, pos{pos}
		{}

		firm::ir_node *node;
		firm::ir_node *pred;
		int pos;
	};

	struct loop_info_counter {
		bool valid{false};
		firm::ir_tarval *initial_value;
		firm::ir_tarval *upper_bound;
		firm::ir_tarval *step;
		firm::ir_relation relation;
		bool decreasing; // set to true, if we have an sub expr(loop_info.loop_cmp) or step is negative, but not both
	};

	struct loop_info {
		firm::ir_loop *loop;
		size_t node_count{0};
		size_t call_count{0};
		size_t phi_count{0};
		size_t branches{0};

		std::vector<loop_edge> head;
		std::vector<loop_edge> tail;

		firm::ir_node *loop_phi;
		firm::ir_node *loop_cmp;

		// loop counter info
		loop_info_counter counter;
	};

	struct copy_walker_env
	{
		copy_walker_env(loop_info &info, size_t index) : info{info}, index{index}
		{}

		loop_info &info;
		size_t index;
	};

	bool is_in_loop(firm::ir_node *node, firm::ir_loop *loop) {
		if (firm::is_Block(node)) {
			return firm::get_irn_loop(node) == loop;
		}
		return firm::get_irn_loop(firm::get_nodes_block(node)) == loop;
	}

	void find_inner_loops(firm::ir_loop *loop, std::vector<firm::ir_loop *> *list) {
		for (size_t i = 0, n = firm::get_loop_n_elements(loop); i < n; i++) {
			auto element = firm::get_loop_element(loop, i);
			if (*element.kind == firm::k_ir_loop) {
				find_inner_loops(element.son, list);
			}
		}
		list->push_back(loop);
	}

	std::vector<firm::ir_loop *> find_loops(firm::ir_graph *irg) {
		auto loops = std::vector<firm::ir_loop *>();
		auto loop = firm::get_irg_loop(irg);
		auto elements = firm::get_loop_n_elements(loop);
		for (size_t i = 0; i < elements; i++) {
			auto element = firm::get_loop_element(loop, i);
			if (*element.kind == firm::k_ir_loop) {
				find_inner_loops(element.son, &loops);
			}
		}
		return loops;
	}

	std::vector<std::pair<int, firm::ir_node*>> get_in_nodes(firm::ir_node *node)
	{
		auto ret = std::vector<std::pair<int, firm::ir_node*>>();
		for (int i = 0, n = firm::get_irn_arity(node); i < n; i++) {
			ret.push_back(std::make_pair(i, firm::get_irn_n(node, i)));
		}
		return ret;
	}

	firm::ir_node* get_loop_cmp(loop_info &info)
	{
		/*
		 *        iv
		 *         |  _____
		 *         | /     |
		 *  end   phi step |
		 *    \   / \ /    |
		 *     \ /  add   /
		 *     cmp  / \__/
		 *      |   |
		 *    cond
		 *     / \
		 *    /   \
		 *  proj proj
		 *
		 */

		// only handle loops, which have one tail(no returns inside the loop) and
		// one head (only one jmp inside the loop - not sure, if we could have anything else)
		if (info.tail.size() != 1 || info.head.size() != 1) {
			std::cout << "invalid head or tail" << std::endl;
			return nullptr;
		}

		auto tail = info.tail[0].pred;
		if (!firm::is_Proj(tail)) {
			return nullptr;
		}
		auto cond = firm::get_Proj_pred(tail);
		auto cmp = firm::get_Cond_selector(cond);
		assert(firm::is_Cmp(cmp));
		return cmp;
	}

	//firm::ir_node* get_node_copy(firm::ir_node *node, size_t index)
	//{
	//	if (copy_node_map.find(node) != copy_node_map.end()) {
	//		if (index < copy_node_map[node]->copies.size()) {
	//			return copy_node_map[node]->copies[index];
	//		}
	//	}
	//	return nullptr;
	//}

	void set_node_copy(firm::ir_node *node, size_t index, firm::ir_node *copy)
	{
		if (copy_node_map.find(node) == copy_node_map.end()) {
			copy_node_map[node] = new copy_node();
		}
		if (index >= copy_node_map[node]->copies.size()) {
			copy_node_map[node]->copies.reserve(static_cast<size_t>(index + 1));
		}
		copy_node_map[node]->copies[index] = copy;
	}

	void free_copy_node_map()
	{
		for (auto &kv : copy_node_map) {
			auto entry = kv.second;
			delete entry;
		}
		copy_node_map.clear();
	}

	void copy_into_loop(firm::ir_node *node, void *env)
	{
		auto env_info = (copy_walker_env*)env;
		// only copy nodes inside the loop
		if (!is_in_loop(node, env_info->info.loop)) {
			return;
		}

		auto irg = firm::get_irn_irg(node);
		auto op = firm::get_irn_op(node);
		auto arity = firm::get_irn_arity(node);
		// get ins for copy
		auto inputs = std::vector<firm::ir_node*>();
		for (int i = 0; i < arity; i++) {
			inputs.push_back(firm::get_irn_n(node, i));
		}
		// create copy
		auto new_node = firm::new_ir_node(
				firm::get_irn_dbg_info(node),
				irg,
				op == firm::op_Block ? nullptr : firm::get_nodes_block(node),
				op,
				firm::get_irn_mode(node),
				arity,
				inputs.data()
		);
		firm::copy_node_attr(irg, node, new_node);
		for (int i = 0; i < arity; i++) {
			if (firm::is_backedge(node, i)) {
				firm::set_backedge(new_node, i);
			}
		}
		if (firm::is_Block(new_node)) {
			firm::set_Block_mark(new_node, 0);
		}
		firm::set_irn_link(node, new_node);
		set_node_copy(node, env_info->index, new_node);
	}

	void rewire_inputs(firm::ir_node *node, void *env)
	{
		auto env_info = (copy_walker_env*)env;
		if (!is_in_loop(node, env_info->info.loop)) {
			return;
		}
		auto copy = (firm::ir_node*)firm::get_irn_link(node);
		// set ins to the copied nodes (if they are inside the loop)
		for (int i = 0, n = firm::get_irn_arity(node); i < n; i++) {
			auto pred = firm::get_irn_n(node, i);
			if (is_in_loop(pred, env_info->info.loop)) {
				firm::set_irn_n(copy, i, (firm::ir_node*)firm::get_irn_link(pred));
			}
		}
		// wire block and phis
		if (!firm::is_Block(node)) {
			auto block = firm::get_nodes_block(node);
			auto cpblock = (firm::ir_node*)firm::get_irn_link(block);
			assert(cpblock);
			firm::set_nodes_block(copy, cpblock);
			if (firm::is_Phi(copy)) {
				firm::add_Block_phi(cpblock, copy);
			}
		}
	}

	void copy_loop(loop_info &info, size_t index)
	{
		firm::inc_irg_visited(current_irg);
		auto env = copy_walker_env(info, index);
		firm::irg_walk_graph(current_irg, copy_into_loop, rewire_inputs, &env);
		//firm::irg_walk_topological(current_irg, copy_walker, &env);
	}

	void do_unroll(loop_info &info)
	{
		size_t counter = 0;
		auto val = info.counter.initial_value;
		copy_node_map = std::map<firm::ir_node*, copy_node*>();

		while (firm::tarval_cmp(val, info.counter.upper_bound) & info.counter.relation) {
			// break, if we reach max loop deep - shouldn't never happen
			assert(counter++ <= MAX_LOOP_ITERATIONS);
			//std::cout << "counter: " << firm::get_tarval_long(val) << std::endl;
			//
			auto new_val = firm::is_Add(info.loop_cmp)
					? firm::tarval_add(val, info.counter.step)
					: firm::tarval_sub(val, info.counter.step);

			// create the copy
			copy_loop(info, counter);

			val = new_val;
		}

		free_copy_node_map();
	}

	//firm::ir_tarval* get_next(loop_info &info, firm::ir_tarval *current)
	//{
	//	if (!(firm::tarval_cmp(current, info.counter.upper_bound) & info.counter.relation)) {
	//		return nullptr;
	//	}
	//
	//	auto next = info.counter.direction == up ?
	//                firm::tarval_add(current, info.counter.step) :
	//                firm::tarval_sub(current, info.counter.step);
	//
	//	return next;
	//}

	//void copy_loop()
	//{
	//
	//}

	long is_const_loop(loop_info &info)
	{
		auto cmp = get_loop_cmp(info);
		if (!cmp) {
			return 0;
		}

		info.loop_cmp = cmp;
		auto lhs = firm::get_Cmp_left(cmp);
		auto rhs = firm::get_Cmp_right(cmp);
		firm::ir_node* cons;
		firm::ir_node* phi;

		// either lhs or rhs have to be constant
		if (firm::is_Const(lhs)) {
			cons = lhs;
			phi = rhs;
		} else if (firm::is_Const(rhs)) {
			cons = rhs;
			phi = lhs;
		} else {
			return 0;
		}

		// one cmp input must be an phi, with two inputs
		// a const (initial value) and an expr
		if (!firm::is_Phi(phi) || firm::get_Phi_n_preds(phi) != 2) {
			return 0;
		}
		info.loop_phi = phi;
		firm::ir_node* iv;
		firm::ir_node* expr;
		if (firm::is_Const(firm::get_Phi_pred(phi, 0))) {
			iv = firm::get_Phi_pred(phi, 0);
			expr = firm::get_Phi_pred(phi, 1);
		} else if (firm::is_Const(firm::get_Phi_pred(phi, 1))) {
			iv = firm::get_Phi_pred(phi, 1);
			expr = firm::get_Phi_pred(phi, 0);
		} else {
			return 0;
		}

		info.counter.initial_value = firm::get_Const_tarval(iv);
		info.counter.upper_bound = firm::get_Const_tarval(cons);
		info.counter.relation = firm::get_Cmp_relation(cmp);
		// we assume, that rhs is the const node: cmp(x < const) - if not, we have to inverse the relation
		if (lhs == cons) {
			info.counter.relation = firm::get_inversed_relation(info.counter.relation);
		}
		// only use simple counting loops (no complex expressions!)
		if (!firm::is_Sub(expr) && !firm::is_Add(expr)) {
			return 0;
		}
		if (firm::is_Const(firm::get_irn_n(expr, 0))) {
			if (firm::get_irn_n(expr, 1) != phi) {
				return 0;
			}
			info.counter.step = firm::get_Const_tarval(firm::get_irn_n(expr, 0));
		} else if (firm::is_Const(firm::get_irn_n(expr, 1))) {
			if (firm::get_irn_n(expr, 0) != phi) {
				return 0;
			}
			info.counter.step = firm::get_Const_tarval(firm::get_irn_n(expr, 1));
		} else {
			return 0;
		}
		info.counter.valid = true;

		auto diff_tar = firm::tarval_sub(info.counter.upper_bound, info.counter.initial_value);
		// we need at least count_tar steps - maybe more
		auto count_tar = firm::tarval_div(diff_tar, info.counter.step);
		if ((firm::is_Sub(expr)) ^ firm::tarval_is_negative(info.counter.step)) {
			info.counter.decreasing = true;
			count_tar = firm::tarval_neg(count_tar);
		}

		if (!(firm::tarval_cmp(info.counter.initial_value, info.counter.upper_bound) & info.counter.relation)) {
			// loop would never taken
			return 0;
		}

		//std::cout << "count: " << firm::get_tarval_long(count_tar) << std::endl;
		//std::cout << "from" << firm::get_tarval_long(info.counter.initial_value) << std::endl;
		//std::cout << "upper" << firm::get_tarval_long(info.counter.upper_bound) << std::endl;
		if (firm::tarval_is_negative(count_tar)) {
			// should never be true
			return 0;
		}

		return firm::get_tarval_long(count_tar);
	}

	void collect_loop_info(firm::ir_node *node, void *env) {
		auto info = (loop_info *) env;
		auto node_in_loop = is_in_loop(node, info->loop);
		// count nodes in loop
		if (node_in_loop) {
			if (firm::is_Call(node)) {
				info->call_count++;
				info->node_count++;
			} else if (firm::is_Phi(node)) {
				info->phi_count++;
				info->node_count++;
			} else if (!is_nop(node)) {
				info->node_count++;
			}
		}

		for (auto &edge : get_in_nodes(node)) {
			auto pred_in_loop = is_in_loop(edge.second, info->loop);
			if (firm::is_Block(node)) {
				// count branches
				size_t outs{0};
				for (auto &edge : get_block_succ(node)) {
					if (firm::is_Block(edge.first) && is_in_loop(edge.first, info->loop)) {
						outs++;
					}
				}
				if (outs > 0) {
					info->branches++;
				}

				if (node_in_loop && !pred_in_loop) {
					info->head.push_back(loop_edge(node, edge.second, edge.first));
				} else if (!node_in_loop && pred_in_loop) {
					info->tail.push_back(loop_edge(node, edge.second, edge.first));
				}
			}
		}
	}

	void optimize_loop(firm::ir_graph *irg, firm::ir_loop *loop) {
		auto info = loop_info();
		info.loop = loop;

		firm::irg_walk_graph(irg, collect_loop_info, nullptr, &info);

		// only one head allowed
		if (info.head.size() != 1) {
			return;
		}
		if (info.branches > MAX_LOOP_BRANCHES
		    || info.node_count > MAX_LOOP_SIZE) {
			return;
		}

		auto unroll_nr = is_const_loop(info);
		if (unroll_nr > 0 && unroll_nr <= static_cast<long>(MAX_LOOP_ITERATIONS)) {
			std::cout << "found loop" << std::endl;
			do_unroll(info);
			return;
		}
	}
}

bool unroll::optimize(firm_ir &) {
	size_t n = firm::get_irp_n_irgs();
	for (size_t i = 0; i < n; i++) {
		auto irg = firm::get_irp_irg(i);
		current_irg = irg;

		firm::assure_irg_properties(irg, firm::IR_GRAPH_PROPERTY_CONSISTENT_LOOPINFO);
		firm::edges_activate(irg);

		firm::ir_reserve_resources(irg, firm::IR_RESOURCE_IRN_LINK | firm::IR_RESOURCE_PHI_LIST);
		firm::collect_phiprojs_and_start_block_nodes(irg);

		auto loops = find_loops(irg);
		for (auto loop : loops) {
			optimize_loop(irg, loop);
			//assert(false);
		}

		firm::edges_deactivate(irg);
		firm::ir_free_resources(irg, firm::IR_RESOURCE_IRN_LINK | firm::IR_RESOURCE_PHI_LIST);
	}
	return false;
}