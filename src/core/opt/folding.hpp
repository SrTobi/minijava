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
		class folding : public worklist_optimization
		{
		public:

			virtual bool handle(firm::ir_node* node);
			virtual void cleanup(firm::ir_node* node);
		};
	}
}