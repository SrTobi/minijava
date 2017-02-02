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
		class control_flow : public optimization
		{
		private:
			bool _changed{false};

			bool try_merge(firm::ir_node *block, int pos);
			bool optimize_block(firm::ir_node *block);

		public:
			virtual bool optimize(firm_ir &) override;
		};
	}
}