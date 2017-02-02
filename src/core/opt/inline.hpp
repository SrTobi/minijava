/**
 * @file inline.hpp
 *
 * @brief
 *     Simple function call inliner.
 *
 */

#pragma once

#include "opt/opt.hpp"

namespace minijava
{
	namespace opt
	{
		class inliner : public optimization
		{
		public:

			virtual bool optimize(firm_ir &) override;

			struct call_node_info {
				int benefice{0}; // benefice for inlineing
				int depth{0}; // inlining depth
				firm::ir_node* call;

				call_node_info(const call_node_info&) = default;

				// default constructor
				call_node_info(firm::ir_node* call) : call{call}
				{
					assert(firm::is_Call(call));
				}

				/**
				 * Search predicate.
				 * We use the `call` pointer to compare, if two `call_node_info` objects are equal.
				 *
				 * @param i
				 * @return
				 */
				bool operator()(const call_node_info& i) const
				{
					return i.call == call;
				}
			};

			// inline info for an irg
			struct irg_inline_info {
				int blocks{0}; // number of blocks
				int nodes{0}; // number of nodes
				int calls{0}; // number of call nodes in the graph
				int callers{0}; // number of callers
				bool got_inlined{false};
				int inline_count{0};
				bool self_recursive{false}; // true, if the irg has a call, which calls itself
				std::vector<inliner::call_node_info> call_nodes;
			};

		private:
			bool _changed{false};
			std::set<firm::ir_node*> _call_to_ignore;

			void collect_irg_calls(firm::ir_graph* irg);

			std::vector<std::pair<firm::ir_graph*, inliner::irg_inline_info>> get_irgs();
			//void release_irgs(std::vector<firm::ir_graph*> irgs);

			void inline_into(firm::ir_graph *irg);

			/**
			 * @brief
			 *     Does the actual inlining.
			 * @param call
			 *     Call node to inline
			 * @param called_irg
			 *     `ir_graph` which should replace `call`
			 * @return
			 *     returns true, if the call got inlined
			 */
			bool inline_method(firm::ir_node *call, firm::ir_graph *called_irg);

			/**
			 * @brief
			 *     Should the given call, which is specified by an call_node_info, be inlined?
			 * @param info
			 *     call_node_info for a given call node.
			 *     Also contains information about call depth and benefice
			 * @return
			 *     true, if the call should be inlined
			 */
			bool should_inline(inliner::call_node_info &info);

			/**
			 * @brief Is called, after a `call` was inlined into `irg`. This method modifies maybe the benefice value of
			 *     `info`.
			 * @param irg
			 *     graph where `call` is inlined
			 * @param call
			 *     inlined function
			 * @param info
			 *     `call_node_info` to modify
			 */
			void maybe_modify_benefice(firm::ir_graph *irg, call_node_info &call, call_node_info &info);

		};
	}
}