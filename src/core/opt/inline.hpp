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
		/**
		 * @brief
		 *     Optimization which tries to inline call nodes to prevent the call at runtime
		 *     and for better execution of other optimizations.
		 */
		class inliner : public optimization
		{
		public:

			/**
			 * @brief
			 *     Runs the optimization on the given irp.
			 * @return
			 */
			virtual bool optimize(firm_ir &) override;

			/**
			 * @brief
			 *     Stores information about a given call node.
			 */
			struct call_node_info {
				/**
				 * benefice for inlineing
				 */
				int benefice{0};

				/**
				 * inlining depth
				 */
				int depth{0};

				/**
				 * The call node
				 */
				firm::ir_node* call;

				/**
				 * Default copy constructor
				 */
				call_node_info(const call_node_info&) = default;

				/**
				 * default constructor which takes a call node
				 * @param call
				 */
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

			/**
			 * @brief
			 *     Inline info for an irg
			 */
			struct irg_inline_info {
				/**
				 * Number of blocks
				 */
				int blocks{0};

				/**
				 * Number of nodes
				 */
				int nodes{0};

				/**
				 * Number of call nodes in the graph
				 */
				int calls{0};

				/**
				 * Number of callers
				 */
				int callers{0};

				/**
				 * True, if irg got already inlined somewhere
				 */
				bool got_inlined{false};

				/**
				 * Number of inlines.
				 */
				int inline_count{0};

				/**
				 * True, if the irg has a call, which calls itself
				 */
				bool self_recursive{false};

				/**
				 * List of all call nodes in the irg
				 */
				std::vector<inliner::call_node_info> call_nodes;
			};

		private:

			/**
			 * @brief
			 *     A list of call nodes, which should not be handled.
			 *     Useful, if you call this optimization more than once.
			 */
			std::set<firm::ir_node*> _call_to_ignore;

			/**
			 * @brief
			 *     Collects all calls of an irg and stores the info inside the call's link
			 * @param irg
			 */
			void collect_irg_calls(firm::ir_graph* irg);

			/**
			 * @brief
			 *     Collects a list of irgs in order and reserves memory for info.
			 * @return
			 */
			std::vector<std::pair<firm::ir_graph*, inliner::irg_inline_info>> get_irgs();

			/**
			 * @brief
			 *     Runs the inliner for the given graph.
			 * @param irg
			 */
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
