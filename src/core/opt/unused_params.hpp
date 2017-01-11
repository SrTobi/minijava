/**
 * @file unused_params.hpp
 *
 * @brief
 *     Remove unused parameters from function calls.
 *
 */

#pragma once

#include "opt/opt.hpp"

namespace minijava
{
	namespace opt
	{
		class unused_params : public optimization
		{
		public:
			virtual bool optimize(firm_ir &ir) override;
		};
	}
}