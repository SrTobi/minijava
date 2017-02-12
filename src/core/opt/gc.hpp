/**
 * @file load_store.hpp
 *
 * @brief
 *     Static garbage collecton optimization.
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
		 *     Remove unused objects
		 */
		class gc : public worklist_optimization
		{
		public:

			/**
			 * @brief
			 *     Remove unused objects
			 * @param node
			 * @return if the graph changed
			 */
			virtual bool handle(firm::ir_node* node);
		};
	}
}
