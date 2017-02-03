/**
 * @file opt.hpp
 *
 * @brief
 *     Interface for optimizations.
 *
 */

#pragma once

#include <queue>

#include "irg/irg.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Namespace for optimizations.
	 */
	namespace opt
	{
		/**
		 * @brief
		 *     Base class all optimizations should inherit from and implement
		 */
		class optimization
		{
		public:
			/**
			 * @brief
			 *     Optimizes the given Firm IRG.
			 *     Should be overwritten.
			 * @param ir
			 *     IRG to optimize
			 * @return
			 *     Returns true, if something has changed in the IRG, otherwhise false
			 */
			virtual bool optimize(firm_ir& /*ir*/) = 0;
		};

		using worklist_queue = std::queue<firm::ir_node*>;

		class worklist_optimization : public optimization
		{
		protected:

			firm::ir_graph* _irg;
			bool _changed{};

		public:
			virtual bool handle(firm::ir_node* node) = 0;
			virtual void cleanup(firm::ir_node* node);

			virtual bool optimize(firm_ir &ir) override;
		};

		class worklist
		{
		protected:
			firm::ir_graph* _irg;

		public:
			worklist(firm::ir_graph* irg);

			void run(worklist_optimization *opt);
		};

		// functions
		std::vector<std::pair<firm::ir_node*, int>> get_out_edges_safe(firm::ir_node* node);
		std::vector<std::pair<firm::ir_node*, int>> get_block_succ(firm::ir_node* node);
		firm::ir_node* copy_irn_to_irg(firm::ir_node *n, firm::ir_graph *irg);
		void clone_irg(firm::ir_graph* from, firm::ir_graph* to);

		/**
		 * @brief
		 *     Has the given ir_node any runtime influence or does it behave like a nop
		 * @param node
		 *     ir_node to check it's opcode
		 * @return
		 *     true, if the node behaves like a nop
		 */
		bool is_nop(firm::ir_node* node);

		firm::ir_tarval* get_tarval(firm::ir_node* node, int n);
		bool is_tarval_numeric(firm::ir_tarval* val);
		bool is_tarval_with_num(firm::ir_tarval* val, long num);
	}

	/**
	 * @brief
	 *     Optimizes the given Firm IRG.
	 *
	 * @param ir
	 *     IRG to optimize
	 *
	 */
	void optimize(firm_ir& ir);

	/**
	 * @brief
	 *     Registers all known optimizatios to be evaluated before running the backend
	 */
	void register_all_optimizations();

	/**
	 * @brief
	 *     Registers a single optimization to be evaluated before running the backend
	 * @param opt
	 *     Optimization to be evaluated
	 */
	void register_optimization(std::unique_ptr<minijava::opt::optimization> opt);

}  // namespace minijava
