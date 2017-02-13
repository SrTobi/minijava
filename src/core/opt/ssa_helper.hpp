/**
 * @file ssa_helper.hpp
 *
 * @brief
 *     adds an empty jmp block, if the projs of an cond lead to the same block
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
		 *     adds an empty jmp block, if the projs of an cond lead to the same block
		 */
		class ssa_helper: public optimization
		{
		public:
			/**
			 * @brief
			 *     Searches in all irg's for tail recursion and tries to replace them
			 * @return
			 */
			virtual bool optimize(firm_ir &) override;
		};
	}
}
