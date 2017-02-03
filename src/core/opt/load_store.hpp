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
		/**
		 * @brief
		 *     Optimize load and store nodes.
		 */
		class load_store : public worklist_optimization
		{
		public:

			/**
			 * @brief
			 *     Optimize load and store nodes. Removes useless nodes.
			 * @param node
			 * @return
			 */
			virtual bool handle(firm::ir_node* node);
		};
	}
}