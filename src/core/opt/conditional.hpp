/**
 * @file conditional.hpp
 *
 * @brief
 *     Conditional optimization.
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
		 *     Optimize the usage of Phi, Mux, Cmp and Cond nodes.
		 *     Replace unused nodes and useless Conds.
		 */
		class conditional : public worklist_optimization
		{
		public:

			/**
			 * @brief
			 *     Calculates for all nodes the known compile time values.
			 * @param node
			 * @return
			 */
			virtual bool handle(firm::ir_node* node);

			/**
			 * @brief
			 *     Exchange Mux and Cond-Projs with their known value.
			 * @param node
			 */
			virtual void cleanup(firm::ir_node* node);
		};
	}
}
