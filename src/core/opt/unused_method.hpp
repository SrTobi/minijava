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
		class unused_method : public optimization
		{
		public:
			virtual bool optimize(firm_ir &ir) override;
		};
	}
}