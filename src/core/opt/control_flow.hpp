/**
 * @file control_flow.hpp
 *
 * @brief
 *     control flow optimization.
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
		 *     Optimization, which optimizes the control flow and removes
		 *     useless blocks and jumps.
		 */
		class control_flow : public optimization
		{
		private:
			/**
			 * Stores, if the optimization changed something
			 */
			bool _changed{false};

			/**
			 * @brief
			 *     Optimizes the control flow of the given block node.
			 * @param block
			 * @return
			 */
			bool optimize_block(firm::ir_node *block);

		public:
			/**
			 * @brief
			 *     Optimizes the control flow of the given irp.
			 * @return
			 */
			virtual bool optimize(firm_ir &) override;
		};
	}
}