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
			virtual bool optimize(firm_ir& ir) = 0;

			/**
			 * @brief
			 *     Virtual default destructor.
			 *
			 */
			virtual ~optimization() = default;
		};

		/**
		 * Type of the worklist queue
		 */
		using worklist_queue = std::queue<firm::ir_node*>;

		/**
		 * @brief
		 *     Base for all worklist based optimization. Calls `handle` and
		 *     `cleanup` in topological order.
		 */
		class worklist_optimization : public optimization
		{
		protected:

			/**
			 * The current irg
			 */
			firm::ir_graph* _irg;

			/**
			 * Should be set, if something changed
			 */
			bool _changed{};

		public:
			/**
			 * @brief
			 *     Called in topological order for each node of each graph.
			 *     Virtual function which must be overridden by the child class.
			 * @param node
			 * @return
			 */
			virtual bool handle(firm::ir_node* node) = 0;

			/**
			 * @brief
			 *     Called in topological order for each node of each graph, AFTER `handle`.
			 *     Virtual function which could be overridden by the child class.
			 * @param node
			 */
			virtual void cleanup(firm::ir_node* node);

			/**
			 * @brief
			 *     Runs the optimization on the given irp.
			 *     Virtual function, which could be overriden by the child class.
			 * @param ir
			 * @return true, if something has changed
			 */
			virtual bool optimize(firm_ir &ir) override;
		};

		/**
		 * @brief
		 *     Runs an worklist based optimization on the given irg.
		 */
		class worklist
		{
		protected:
			/**
			 * The current irg.
			 */
			firm::ir_graph* _irg;

		public:
			/**
			 * @brief
			 *     Constructor, expects and irg
			 * @param irg
			 */
			worklist(firm::ir_graph* irg);

			/**
			 * @brief
			 *     Runs the worklist optimization.
			 * @param opt
			 */
			void run(worklist_optimization *opt);
		};


		/**
		 * @brief
		 *     Get all out edges of the given `node`.
		 *     Expects, that `firm::edges_activate` is called before.
		 * @param node
		 * @return
		 */
		std::vector<std::pair<firm::ir_node*, int>> get_out_edges_safe(firm::ir_node* node);

		/**
		 * @brief
		 *     Returns the successor of the given block
		 * @param node
		 * @return
		 */
		std::vector<std::pair<firm::ir_node*, int>> get_block_succ(firm::ir_node* node);

		/**
		 * @brief
		 *     Copies the given node `n` to `irg` and returns the new node.
		 * @param n
		 * @param irg
		 * @return
		 */
		firm::ir_node* copy_irn_to_irg(firm::ir_node *n, firm::ir_graph *irg);


		/**
		 * @brief
		 *     Has the given ir_node any runtime influence or does it behave like a nop
		 * @param node
		 *     ir_node to check it's opcode
		 * @return
		 *     true, if the node behaves like a nop
		 */
		bool is_nop(firm::ir_node* node);

		/**
		 * @brief
		 *     Returns true, if the given tarval is numeric
		 * @param val
		 * @return
		 */
		bool is_tarval_numeric(firm::ir_tarval* val);

		/**
		 * @brief
		 *     Returns true, if the given tarval `val` has the given long value `num`
		 * @param val
		 * @param num
		 * @return
		 */
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

	/**
	 * @brief
	 *     Registers a single optimization by name to be evaluated before running the backend
	 *
	 * @param opt
	 *     Name of the optimization to be evaluated
	 */
	void register_optimization(const std::string& opt);

	/**
	 * @brief
	 *     Returns the names of all optimizations.
	 *
	 * The names are sorted in the recommanded order for applying the optimizations.
	 *
	 * @return
	 *     the names of all known optimizations
	 */
	const std::vector<std::string>& get_optimization_names();


}  // namespace minijava
