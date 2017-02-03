/**
 * @file folding.hpp
 *
 * @brief
 *     Constant folder optimization.
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
		 *     Constant folder optimization.
		 *     Tries to calculate the compile time known value of expressions
		 *     to prevent the calculation at runtime.
		 */
		class folding : public worklist_optimization
		{
		public:

			/**
			 * @brief
			 *     Tries to calculate the value of the given node. As this method
			 *     is called in topological order, all preds are calculated before
			 *     visiting the given node.
			 * @param node
			 * @return
			 */
			virtual bool handle(firm::ir_node* node);

			/**
			 * @brief
			 *     Replace the given node with the constant value (if previously set in `handle`).
			 * @param node
			 */
			virtual void cleanup(firm::ir_node* node);
		};
	}
}