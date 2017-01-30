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
		class conditional : public worklist_optimization
		{
		public:

			virtual bool handle(firm::ir_node* node);
			virtual void cleanup(firm::ir_node* node);
		};
	}
}