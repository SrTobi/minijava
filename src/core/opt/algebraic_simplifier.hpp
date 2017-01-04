/**
 * @file algebraic_simplifier.hpp
 *
 * @brief
 *     Algebraic simplifier optimization.
 *
 */

#pragma once

#include "opt/opt.hpp"

namespace minijava
{
	namespace opt
	{
		class algebraic_simplifier : public worklist_optimization
		{
		public:

			virtual bool handle(firm::ir_node* node);
			virtual void cleanup(firm::ir_node* node);
		};
	}
}