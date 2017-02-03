/**
 * @file unused_method.hpp
 *
 * @brief
 *     Remove unused methods from irp.
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
		 *    Remove unused methods from irp.
		 */
		class unused_method : public optimization
		{
		public:
			/**
			 * @brief
			 *     Runs the optimization on the given ir.
			 * @param ir
			 * @return
			 */
			virtual bool optimize(firm_ir &ir) override;
		};
	}
}