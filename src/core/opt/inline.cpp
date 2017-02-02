#include "opt/inline.hpp"

#include <iostream>
#include <vector>

#define foreach_irn_in(irn, idx, pred) \
	for (bool pred##__b = true; pred##__b;) \
		for (firm::ir_node const *const pred##__irn = (irn); pred##__b; pred##__b = false) \
			for (int idx = 0, pred##__n = firm::get_irn_arity(pred##__irn); pred##__b && idx != pred##__n; ++idx) \
				for (firm::ir_node *const pred = (pred##__b = false, firm::get_irn_n(pred##__irn, idx)); !pred##__b; pred##__b = true)

using namespace minijava::opt;

// TODO: find useful values for this two consts
const int max_nodes = 1024;
const int max_inline_count = 100;
const int inline_threshold = 1024;

static firm::ir_graph* current_irg;

struct walker_env {
	inliner::irg_inline_info* info;
	firm::ir_graph* irg;
};

/**
 * Calculates the benefice, to inline a specific call node
 * @param call_node
 *     call node to calculate the benefice
 * @return
 *     benefice to inline the call node - the bigger, the better
 */
int get_benefice(firm::ir_node* call_node)
{
	assert(firm::is_Call(call_node));
	auto irg = firm::get_entity_irg(firm::get_Call_callee(call_node));
	auto info = (inliner::irg_inline_info*)firm::get_irg_link(irg);
	int benefice = firm::get_Call_n_params(call_node) * 5;

	bool all_const = true;
	for (int i = 0, n = firm::get_Call_n_params(call_node); i < n; i++) {
		auto param = firm::get_Call_param(call_node, i);
		if (firm::is_Const(param)) {
			benefice += 100;
		} else {
			all_const = false;
		}
	}
	// all params are constant?
	if (all_const) {
		benefice += 1024;
	}
	// only one block in the irg?
	if (info->blocks == 1) {
		benefice = benefice * 3 / 2;
	}
	// small graph
	if (info->nodes < 30) {
		benefice += 2000;
	}
	// no calls is also good
	if (info->calls == 0) {
		benefice += 400;
	}
	return benefice;
}

/**
 * @brief
 *     Simple check against a call node with, if it could be inlined.
 * @param call
 *     The call node, which should be inlined
 * @return
 *     True, if it could be inlined
 */
bool can_inline(firm::ir_node* call)
{
	assert(firm::is_Call(call));
	auto call_entity = firm::get_Call_callee(call);
	auto called_irg = firm::get_entity_irg(call_entity);
	auto called_entity = firm::get_irg_entity(called_irg);
	auto called_type = firm::get_entity_type(called_entity);
	auto call_type = firm::get_entity_type(call_entity);
	// parameter count should be equal
	// assert this shouldn't never happen in our compiler
	auto n_res = firm::get_method_n_ress(called_type);
	assert (n_res == firm::get_method_n_ress(call_type));
	return true;
}

/**
 * @brief
 *     Links a node with it's new copy and mark it as visited
 * @param node
 *     Node
 * @param new_node
 *     New node
 */
void set_new_node(firm::ir_node* node, firm::ir_node* new_node)
{
	firm::set_irn_link(node, new_node);
	firm::mark_irn_visited(node);
}

/**
 * @brief
 *     Returns for a node it's copy.
 * @see
 *     set_new_node
 * @param node
 *     The node
 * @return
 *     The copy
 */
firm::ir_node* get_new_node(firm::ir_node* node)
{
	assert(firm::irn_visited(node));
	return (firm::ir_node*)firm::get_irn_link(node);
}

/**
 * @brief
 *     Creates a copy of the given node.
 *     Used inside irg_walk_core walker function
 * @param node
 *     Node to copy
 * @param env
 *     Pointer to the new `ir_graph`
 */
void copy_node_inline(firm::ir_node *node, void *env)
{
	auto new_irg = (firm::ir_graph*)env;
	auto op = firm::get_irn_op(node);
	auto arity = firm::get_irn_arity(node);
	// get ins for copy
	auto inputs = std::vector<firm::ir_node*>();
	for (int i = 0; i < arity; i++) {
		inputs.push_back((firm::ir_node*)firm::get_irn_link(firm::get_irn_n(node, i)));
	}
	// create copy
	auto new_node = firm::new_ir_node(
			firm::get_irn_dbg_info(node),
	        new_irg,
	        op == firm::op_Block ? nullptr : firm::get_nodes_block(node),
	        op,
	        firm::get_irn_mode(node),
	        arity,
	        inputs.data()
	);
	firm::copy_node_attr(new_irg, node, new_node);
	firm::set_irn_link(node, new_node);
}

/**
 * @brief
 *     Rewires the ins and block of the newly created node with its preds.
 *     Used inside irg_walk_core walker function.
 * @param node
 *     Node to set it's ins
 * @see
 *     set_preds_inline
 */
void rewire_inputs(firm::ir_node* node)
{
	auto new_node = get_new_node(node);
	// set block of non block nodes
	if (!firm::is_Block(node)) {
		auto block = firm::get_nodes_block(node);
		auto new_block = get_new_node(block);
		firm::set_nodes_block(new_node, new_block);
	}
	// set ins
	foreach_irn_in(node, i, in) {
					auto new_in = get_new_node(in);
					firm::set_irn_n(new_node, i, new_in);
				}
}

/**
 * @brief
 *     Sets the ins and block if the given node.
 * @param node
 *     Node to set the ins and block
 * @param env
 *     Pointer to the new `ir_graph`
 */
void set_preds_inline(firm::ir_node *node, void *env)
{
	rewire_inputs(node);
	// move consts into start block
	auto new_node = get_new_node(node);
	if (firm::is_irn_start_block_placed(new_node)) {
		auto new_irg = (firm::ir_graph*)env;
		auto start_block = firm::get_irg_start_block(new_irg);
		firm::set_nodes_block(new_node, start_block);
	}
}

/**
 * @brief
 *     Walker function, which collects information about the structure of a given `ir_graph`
 * @param node
 *     Current node
 * @param env
 *     Contains a pointer to an `walker_env`
 */
void collect_calls(firm::ir_node* node, void* env)
{
	auto w_env = (walker_env*)env;
	auto info = w_env->info;


	// nodes without any cost could be ignored
	if (is_nop(node)) {
		return;
	}

	if (firm::is_Block(node)) {
		info->blocks++;
	} else {
		info->nodes++;
	}

	if (!firm::is_Call(node)) {
		return;
	}
	info->calls++;
	auto callee_ent = firm::get_Call_callee(node);
	if (!callee_ent) {
		return;
	}
	auto callee = firm::get_entity_irg(callee_ent);
	if (!callee) {
		return;
	}
	auto callee_info = (inliner::irg_inline_info *) firm::get_irg_link(callee);
	if (callee_info) {
		callee_info->callers++;
	}
	if (w_env->irg == callee) {
		info->self_recursive = true;
	}
	auto call_info = inliner::call_node_info(node);
	call_info.benefice = get_benefice(call_info.call);
	info->call_nodes.push_back(call_info);
}

void inliner::maybe_modify_benefice(firm::ir_graph* /*irg*/, call_node_info& /*call*/, call_node_info& /*info*/)
{
	// currently don't do anything - maybe add some logic later
}

bool inliner::should_inline(call_node_info &info)
{
	assert(firm::is_Call(info.call));
	auto callee = firm::get_Call_callee(info.call);
	auto callee_type = firm::get_entity_type(callee);
	auto properties = firm::get_method_additional_properties(callee_type);

	if (_call_to_ignore.find(info.call) != _call_to_ignore.end()) {
		return false;
	}

	// methods with always_inline property should be always inlined
	if (properties & firm::mtp_property_always_inline) {
		return true;
	}
	// method has noinline property
	if (properties & firm::mtp_property_noinline) {
		return false;
	}
	// recursion depth of inlined call is too deep
	if (info.depth > 20) {
		return false;
	}
	// benefice of inline this method is higher than our threshold
	if (info.benefice > inline_threshold) {
		return true;
	}

	return false;
}

bool inliner::inline_method(firm::ir_node *call, firm::ir_graph *called_irg)
{
	if (!can_inline(call)) {
		return false;
	}

	auto irg = firm::get_irn_irg(call);
	// We don't inline irgs in itself
	assert(irg != called_irg);
	assert(firm::get_compound_n_members(firm::get_irg_frame_type(called_irg)) == 0);
	// store current_irg for later
	auto old_irg = current_irg;
	current_irg = irg;

	// count of method params
	auto params = static_cast<int>(firm::get_method_n_params(firm::get_entity_type(firm::get_irg_entity(called_irg))));

	auto post_block = firm::get_nodes_block(call);
	auto args_in = std::vector<firm::ir_node*>();
	for (int i = params - 1; i >= 0; i--) {
		// Attention! Fill args_in in reverse order, because we walk in reverse!
		args_in.push_back(firm::get_Call_param(call, params - i - 1));
	}

	firm::ir_node* in[3];
	in[firm::pn_Start_M] = firm::get_Call_mem(call);
	in[firm::pn_Start_P_frame_base] = firm::get_irg_frame(irg);
	in[firm::pn_Start_T_args] = firm::new_r_Tuple(post_block, params, args_in.data());
	auto pre_call = firm::new_r_Tuple(post_block, 3, in);

	// split block into two
	firm::part_block(pre_call);

	firm::inc_irg_visited(called_irg);

	// use pre_call for start and end blocks
	set_new_node(firm::get_irg_start_block(called_irg), firm::get_nodes_block(pre_call));
	set_new_node(firm::get_irg_start(called_irg), pre_call);
	set_new_node(firm::get_irg_no_mem(called_irg), firm::get_irg_no_mem(irg));

		// copy called_irg to irg
	assert(firm::get_compound_n_members(firm::get_irg_frame_type(called_irg)) == 0);
	firm::irg_walk_core(firm::get_irg_end(called_irg), copy_node_inline, set_preds_inline, irg);

	auto end_block = get_new_node(firm::get_irg_end_block(called_irg));
	auto end = get_new_node(firm::get_irg_end(called_irg));
	auto ctp = firm::get_Call_type(call);
	auto arity = firm::get_irn_arity(end_block);
	auto n_res = firm::get_method_n_ress(ctp);
	assert(arity > 0);

	// keep keepalives
	for (int i = 0, irn_arity = firm::get_irn_arity(end); i < irn_arity; i++) {
		auto ka = firm::get_End_keepalive(end, i);
		assert(!firm::is_Bad(ka));
		firm::add_End_keepalive(firm::get_irg_end(irg), ka);
	}

	// replace return nodes with jump nodes
	auto post_block_in = std::vector<firm::ir_node*>();
	auto mem_preds = std::vector<firm::ir_node*>();
	for (int i = 0; i < arity; i++) {
		auto ret = firm::get_irn_n(end_block, i);
		// we only should have return nodes going to the end block
		assert(firm::is_Return(ret));
		post_block_in.push_back(firm::new_r_Jmp(firm::get_nodes_block(ret)));
		mem_preds.push_back(firm::get_Return_mem(ret));
	}
	firm::set_irn_in(post_block, arity, post_block_in.data());

	auto call_mem = arity == 1 ? mem_preds[0] :
	                firm::new_r_Phi(post_block, arity, mem_preds.data(), firm::mode_M);

	auto res_pred = std::vector<firm::ir_node*>();
	firm::ir_node* call_res;
	if (n_res > 0) {
		for (size_t i = 0; i < n_res; i++) {
			auto res_type = firm::get_method_res_type(ctp, i);
			auto res_mode = firm::get_type_mode(res_type);

			auto return_nodes = std::vector<firm::ir_node*>();
			for (int j = 0; j < arity; j++) {
				auto ret = firm::get_irn_n(end_block, j);
				assert(firm::is_Return(ret));
				auto res = firm::get_Return_res(ret, static_cast<int>(i));
				assert(firm::get_irn_mode(res) == res_mode);
				return_nodes.push_back(res);
			}

			auto phi = arity == 1 ? return_nodes[0] :
			           firm::new_r_Phi(post_block, arity, return_nodes.data(), res_mode);

			res_pred.push_back(phi);
		}
		call_res = firm::new_r_Tuple(post_block, static_cast<int>(n_res), res_pred.data());
	} else {
		call_res = firm::new_r_Bad(irg, firm::mode_T);
	}

	firm::ir_node *call_in[2];
	call_in[firm::pn_Call_M] = call_mem;
	call_in[firm::pn_Call_T_result] = call_res;
	firm::turn_into_tuple(call, 2, call_in);

	current_irg = old_irg;
	return true;
}

void inliner::inline_into(firm::ir_graph *irg)
{
	auto info = (irg_inline_info*)firm::get_irg_link(irg);
	// to big or no calls inside the graph
	if (info->calls == 0 ||
			info->nodes > max_nodes) {
		return;
	}

	current_irg = irg;

	// get calls to inline
	std::queue<call_node_info> queue;
	for (auto &call_info : info->call_nodes) {
		if (should_inline(call_info)) {
			queue.push(call_info);
		}
	}

	// inline calls
	while (!queue.empty() && info->inline_count < max_inline_count) {
		auto call_info = queue.front();
		queue.pop();

		auto call_node = call_info.call;
		assert(firm::is_Call(call_node));
		auto callee_ent = firm::get_Call_callee(call_node);
		auto callee = firm::get_entity_irg(callee_ent);
		auto callee_env = (irg_inline_info*)firm::get_irg_link(callee);

		if (current_irg == callee) {
			// dont inline recursive calls
			continue;
		}
		// phiproj computation is needed for part_block
		firm::collect_phiprojs_and_start_block_nodes(irg);
		if (!inline_method(call_node, callee)) {
			continue;
		}
		// remove call_node from call_nodes list
		auto pos = std::find_if(info->call_nodes.begin(), info->call_nodes.end(), call_node_info(call_info));
		assert(pos != info->call_nodes.end());
		info->call_nodes.erase(pos);
		info->callers--;
		info->got_inlined = true;
		info->inline_count++;

		// after we inlined callee, there might be some new calls
		for (auto &item : callee_env->call_nodes) {
			call_node_info new_call_info = item;
			auto new_call = get_new_node(new_call_info.call);
			new_call_info.call = new_call;
			if (firm::get_irn_irg(new_call) != irg) {
				continue;
			}
			if (!firm::is_Call(new_call)) {
				// TODO: Shouldn't happen..
				continue;
			}
			auto new_call_irg = firm::get_entity_irg(firm::get_Call_callee(new_call));
			auto new_call_irg_env = (irg_inline_info*)firm::get_irg_link(new_call_irg);

			// prevent recursion
			if (new_call_irg == irg) {
				continue;
			}


			// inc callers of new_call_irg
			new_call_irg_env->callers++;
			// set correct depth of the call
			new_call_info.depth = call_info.depth + 1;
			maybe_modify_benefice(irg, call_info, new_call_info);
			if (should_inline(new_call_info) && !new_call_irg_env->self_recursive) {
				info->call_nodes.push_back(new_call_info);
				queue.push(new_call_info);
			} else {
				// when we shouldn't inline it this time - dont do it later
				_call_to_ignore.insert(new_call);
			}
		}
	}
}

/**
 * Collects all calls of an irg and stores the info inside the call's link
 * @param irg
 */
void inliner::collect_irg_calls(firm::ir_graph* irg)
{
	auto env = walker_env();
	env.irg = irg;
	env.info = (irg_inline_info*)firm::get_irg_link(irg);
	firm::irg_walk_graph(irg, nullptr, collect_calls, &env);
}

/**
 * Collects a list of irgs in order and reserves memory for info.
 * @return
 */
std::vector<std::pair<firm::ir_graph*, inliner::irg_inline_info>> inliner::get_irgs()
{
	auto n_irgs = firm::get_irp_n_irgs();
	auto irgs = std::vector<std::pair<firm::ir_graph*, irg_inline_info>>();

	// needed for compute_callgraph
	firm::ir_entity **free_methods;
	firm::cgana(&free_methods);
	free(free_methods);

	// collect all irgs in order and add irg_inline_info
	firm::compute_callgraph();
	firm::callgraph_walk([](firm::ir_graph* irg, void *env){
		//firm::set_irg_link(irg, &pair.second);
		((std::vector<std::pair<firm::ir_graph*, irg_inline_info>>*)env)->push_back(std::make_pair(irg, irg_inline_info()));
	}, nullptr, &irgs);
	firm::free_callgraph();
	assert(irgs.size() == n_irgs);
	return irgs;
}

bool inliner::optimize(firm_ir &)
{
	bool changed = false;

	auto irgs = get_irgs();

	// link the info with the irgs
	for (auto &kv : irgs) {
		// PHI_LIST is for firm::collect_phiprojs_and_start_block_nodes
		firm::ir_reserve_resources(kv.first, firm::IR_RESOURCE_IRN_LINK | firm::IR_RESOURCE_PHI_LIST);
		firm::set_irg_link(kv.first, &kv.second);
	}

	// collect calls
	for (auto &kv : irgs) {
		collect_irg_calls(kv.first);
	}

	// inline
	for (auto &kv : irgs) {
		inline_into(kv.first);
	}

	// cleanup irgs
	for (auto &kv : irgs) {
		auto irg = kv.first;
		// reset irn links
		firm::irg_walk_graph(irg, [](firm::ir_node *node, void*) {
			firm::set_irn_link(node, nullptr);
		}, nullptr, nullptr);
		firm::ir_free_resources(kv.first, firm::IR_RESOURCE_IRN_LINK | firm::IR_RESOURCE_PHI_LIST);

		// cleanup irg
		firm::remove_tuples(irg);
		firm::remove_bads(irg);
		assert(firm::irg_verify(irg));

		// got something inlined?
		if (kv.second.got_inlined) {
			changed = true;
		}
	}

	return changed;
}