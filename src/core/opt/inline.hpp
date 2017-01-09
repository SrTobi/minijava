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
		class inliner : public worklist_optimization
		{
		public:

			virtual bool handle(firm::ir_node* node);
			virtual void cleanup(firm::ir_node* node);
		};
	}
}