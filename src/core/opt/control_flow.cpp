#include "opt/control_flow.hpp"

#include <iostream>

using namespace minijava::opt;

/**
 * @brief
 *     Used initially to mark all blocks, which only have jmp or phi nodes.
 *     The information is later updated if we merge blocks
 * @param block
 * @param removeable
 */
void set_block_removable(firm::ir_node *block, bool removeable)
{
	assert(firm::is_Block(block));
	firm::set_Block_mark(block, removeable ? 1 : 0);
}

/**
 * @brief
 *     Can the block safely removed?
 * @param block
 * @return
 */
bool is_block_removable(firm::ir_node *block)
{
	return firm::get_Block_mark(block) == 1;
}

/**
 * @brief
 *     Removes "keep alive" edge from irg end node and replace it with a new bad-node.
 * @param node
 */
void remove_keep_alive(firm::ir_node *node)
{
	auto irg = firm::get_irn_irg(node);
	auto end = firm::get_irg_end(irg);
	for (int i = 0, n = firm::get_irn_arity(end); i < n; i++) {
		if (firm::get_irn_n(end, i) == node) {
			firm::set_irn_n(end, i, firm::new_r_Bad(irg, firm::get_irn_mode(node)));
		}
	}
}

/**
 * @brief
 *     Used to exchange phi nodes with oder nodes. If it's a loop, remove also keep alive edges
 * @param old_node
 * @param new_node
 */
void exchange_phi(firm::ir_node *old_node, firm::ir_node *new_node)
{
	if (firm::get_Phi_loop(old_node)) {
		remove_keep_alive(old_node);
		firm::set_Phi_loop(old_node, false);
	}
	firm::exchange(old_node, new_node);
}

/**
 * @brief
 *     Returns a list of all phi's defined in the given block
 * @param block
 * @return
 */
std::vector<firm::ir_node*> get_block_phis(firm::ir_node *block)
{
	assert(firm::is_Block(block));
	auto ret = std::vector<firm::ir_node*>();
	for (auto phi = firm::get_Block_phis(block); phi; phi = firm::get_Phi_next(phi)) {
		ret.push_back(phi);
	}
	return ret;
}

/**
 * @brief
 *     Returns a list of all predecessor nodes of the given block.
 *     Skips all bad nodes
 * @param block
 * @return
 *     list of pairs with index and ir_node
 */
std::vector<std::pair<int, firm::ir_node*>> get_block_preds(firm::ir_node *block)
{
	assert(firm::is_Block(block));
	auto ret = std::vector<std::pair<int, firm::ir_node*>>();
	for (int i = 0, n = firm::get_irn_arity(block); i < n; i++) {
		auto irn = firm::get_irn_n(block, i);
		if (firm::is_Bad(irn)) continue;
		ret.push_back(std::make_pair(i, irn));
	}
	return ret;
}

/**
 * @brief
 *     Updates phi links to their preds, if possible.
 *     Tansformation: phi -> pred -> pred_of_pred => phi -> pred_of_pred
 * @param block
 */
void fix_phi_nodes(firm::ir_node *block)
{
	assert(firm::is_Block(block));
	for (auto phi : get_block_phis(block)) {
		auto in = std::vector<firm::ir_node*>();
		for (auto &pred : get_block_preds(block)) {
			auto pred_block = firm::get_nodes_block(pred.second);
			auto phi_pred = firm::get_irn_n(phi, pred.first);
			auto phi_pred_block = firm::get_nodes_block(phi_pred);

			// non empty or same block as pred: keep it as it is!
			if (pred_block == block || !is_block_removable(pred_block)) {
				in.push_back(phi_pred);
				continue;
			}
			// else:
			for (auto &kv : get_block_preds(pred_block)) {
				// block of phi pred is the same as our pred block? set the input directly
				// transform: phi -> pred -> pred_of_pred => phi -> pred_of_pred
				if (phi_pred_block == pred_block) {
					in.push_back(firm::get_irn_n(phi_pred, kv.first));
				} else {
					// keep it as it is
					in.push_back(phi_pred);
				}
			}
		}

		// only one input left => we dont need a phi node anymore
		if (in.size() == 1) {
			exchange_phi(phi, in[0]);
			firm::set_Block_phis(block, nullptr);
		} else {
			firm::set_irn_in(phi, static_cast<int>(in.size()), in.data());
		}
	}
}

/**
 * @brief
 *     Update the phis of  the pred of the given block
 * @param block
 */
void fix_phi_nodes_pred(firm::ir_node *block)
{
	assert(firm::is_Block(block));
	for (auto &pred_a : get_block_preds(block)) {
		auto pred_a_block = firm::get_nodes_block(pred_a.second);
		if (pred_a_block == block || !is_block_removable(pred_a_block)) continue;

		for (auto phi : get_block_phis(pred_a_block)) {
			auto ins = std::vector<firm::ir_node*>();
			for (auto &pred_b : get_block_preds(block)) {
				auto pred_b_block = firm::get_nodes_block(pred_b.second);
				if (pred_b_block == block || !is_block_removable(pred_b_block)) {
					ins.push_back(phi);
					continue;
				}

				if (pred_a.first == pred_b.first) {
					// add edge to pred
					for (auto &pred_of_pred : get_block_preds(pred_b_block)) {
						ins.push_back(firm::get_irn_n(phi, pred_of_pred.first));
					}
				} else {
					// add self edge
					for (int i = 0, n = firm::get_irn_arity(pred_b_block); i < n; i++) {
						ins.push_back(phi);
					}
				}
			}

			// only one input left => replace phi with input node and clear block phis
			if (ins.size() == 1) {
				exchange_phi(phi, ins[0]);
				firm::set_Block_phis(block, nullptr);
			} else {
				// update phi input and block phis
				firm::set_irn_in(phi, static_cast<int>(ins.size()), ins.data());
				// move phi to this block and update phi list
				firm::set_nodes_block(phi, block);
				firm::set_Phi_next(phi, firm::get_Block_phis(block));
				firm::set_Block_phis(block, phi);
				if (firm::get_Phi_loop(phi)) {
					remove_keep_alive(phi);
					firm::set_Phi_loop(phi, false);
				}
			}
		}
	}
}

void merge_empty_preds(firm::ir_node *block)
{
	assert(firm::is_Block(block));

	fix_phi_nodes(block);
	fix_phi_nodes_pred(block);

	// at this point, we have to update the block input nodes
	// use
	auto ins = std::vector<firm::ir_node*>();
	for (auto &pred : get_block_preds(block)) {
		auto pred_block = firm::get_nodes_block(pred.second);
		// block cant be removed? add it
		if (pred_block == block || !is_block_removable(pred_block)) {
			ins.push_back(pred.second);
			continue;
		}
		// else: block can be removed => use pred blocks as input
		for (auto &pred_of_pred : get_block_preds(pred_block)) {
			ins.push_back(pred_of_pred.second);
		}
		firm::exchange(pred_block, block);
	}
	firm::set_irn_in(block, static_cast<int>(ins.size()), ins.data());
}

/**
 * Remove cond nodes, where true and false leads to the same block
 * @param block
 * @return
 */
bool remove_trivial_cond(firm::ir_node *block)
{
	//auto preds = firm::get_Block_cfgpred_arr(block);
	auto pred_count = static_cast<size_t>(firm::get_Block_n_cfgpreds(block));
	bool found{false};
	// copy ins to vector
	auto preds = std::vector<firm::ir_node*>();
	for (size_t i = 0; i < pred_count; i++) {
		auto irn = firm::get_irn_n(block, static_cast<int>(i));
		preds.push_back(firm::is_Bad(irn) ? nullptr : irn);
	}

	// search for cond nodes, which fork to the same block
	for (size_t i = 0; i < pred_count; i++) {
		// already removed?
		if (preds[i] == nullptr) continue;
		// find first proj for cond node
		auto pred_a = preds[i];
		if (!firm::is_Proj(pred_a)) continue;

		auto proj_pred = firm::get_Proj_pred(pred_a);
		if (!firm::is_Cond(proj_pred)) continue;

		// find other proj for cond node
		for (size_t j = i + 1; j < pred_count; j++) {
			// already removed?
			if (preds[j] == nullptr) continue;
			// get second pred
			auto pred_b = preds[j];
			if (!firm::is_Proj(pred_b)) continue;
			if (proj_pred != firm::get_Proj_pred(pred_b)) continue;
			// found two projs to the same cond node => might be merged
			// first check, if the phi nodes have the same input
			bool same_input = true;
			firm::ir_node *next;
			for (auto phi = firm::get_Block_phis(block); phi != nullptr; phi = next) {
				next = firm::get_Phi_next(phi);
				if (firm::get_Phi_pred(phi, static_cast<int>(i)) != firm::get_Phi_pred(phi, static_cast<int>(j))) {
					same_input = false;
					break;
				}
			}
			if (!same_input) {
				break;
			}
			// now replace pred_a with a jmp and remove pred_b
			preds[i] = firm::new_r_Jmp(firm::get_nodes_block(pred_a));
			preds[j] = nullptr;
			found = true;
		}
	}

	if (found) {
		// only use preds of block, where pred[i] != nullptr
		// first fix phi nodes
		firm::ir_node *next;
		for (auto phi = firm::get_Block_phis(block); phi; phi = next) {
			next = firm::get_Phi_next(phi);
			auto phi_in = std::vector<firm::ir_node*>();
			for (size_t i = 0; i < pred_count; i++) {
				if (preds[i]) {
					phi_in.push_back(firm::get_Phi_pred(phi, static_cast<int>(i)));
				}
			}
			firm::set_irn_in(phi, static_cast<int>(phi_in.size()), phi_in.data());
		}
		// then block preds
		auto in = std::vector<firm::ir_node*>();
		for (size_t i = 0; i < pred_count; i++) {
			if (preds[i]) {
				in.push_back(preds[i]);
			}
		}
		firm::set_irn_in(block, static_cast<int>(in.size()), in.data());
		return true;
	}

	return false;
}

bool try_merge(firm::ir_node *block, int pos)
{
	assert(firm::is_Block(block));
	assert (firm::get_Block_entity(block) == nullptr);
	// only works for preds, which are jmp nodes
	if (!firm::is_Jmp(firm::get_irn_n(block, pos))) {
		return false;
	}

	// replace phi nodes with their input
	auto pred_block = firm::get_Block_cfgpred_block(block, pos);
	for (auto &phi : get_block_phis(block)) {
		exchange_phi(phi, firm::get_Phi_pred(phi, pos));
	}
	// inherit removable property to exchanged block
	if (!is_block_removable(block)) {
		set_block_removable(pred_block, false);
	}
	firm::exchange(block, pred_block);

	return true;
}

bool control_flow::optimize_block(firm::ir_node *block)
{
	assert(firm::is_Block(block));
	if (firm::irn_visited_else_mark(block)) {
		return false;
	}

	size_t preds = 0;
	size_t pred_pos = 0;
	for (int i = 0, n = firm::get_irn_arity(block); i < n; i++) {
		auto pred = firm::get_irn_n(block, i);

		auto pred_block = firm::get_nodes_block(pred);
		if (optimize_block(pred_block)) {
			return false;
		}

		preds++;
		pred_pos = 0;
	}

	if (preds == 1 && try_merge(block, static_cast<int>(pred_pos))) {
		_changed = true;
		return true;
	}

	if (remove_trivial_cond(block)) {
		_changed = true;
		return true;
	}

	merge_empty_preds(block);

	return false;
}

bool control_flow::optimize(firm_ir &)
{
	_changed = false;
	size_t n = firm::get_irp_n_irgs();
	for (size_t i = 0; i < n; i++) {
		auto irg = firm::get_irp_irg(i);
		firm::ir_reserve_resources(irg, firm::IR_RESOURCE_PHI_LIST | firm::IR_RESOURCE_IRN_LINK);

		firm::irg_walk_graph(irg, [](firm::ir_node* node, void*) {
			if (firm::is_Block(node)) {
				set_block_removable(node, true);
				firm::set_Block_phis(node, nullptr);
			} else if (firm::is_Phi(node)) {
				firm::set_Phi_next(node, nullptr);
			}
		}, [](firm::ir_node *node, void*) {
			if (firm::is_Phi(node)) {
				auto block = firm::get_nodes_block(node);
				firm::add_Block_phi(block, node);
				return;
			}

			if (firm::is_Block(node) || firm::is_Jmp(node)) {
				return;
			}
			set_block_removable(firm::get_nodes_block(node), false);
		}, nullptr);

		firm::inc_irg_visited(irg);

		optimize_block(firm::get_irg_end_block(irg));
	}

	// cleanup
	for (size_t i = 0; i < n; i++) {
		auto irg = firm::get_irp_irg(i);
		firm::ir_free_resources(irg, firm::IR_RESOURCE_PHI_LIST | firm::IR_RESOURCE_IRN_LINK);
		firm::remove_unreachable_code(irg);
		firm::remove_bads(irg);
		assert(firm::irg_verify(irg));
	}
	return _changed;
}