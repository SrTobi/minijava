/**
 * @file load_store.hpp
 *
 * @brief
 *     Load/store optimization.
 *
 */

#pragma once

#include "opt/opt.hpp"

namespace minijava
{
	namespace opt
	{
		class load_store : public worklist_optimization
		{
		public:

			virtual bool handle(firm::ir_node* node);
		};
	}
}