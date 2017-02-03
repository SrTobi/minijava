/**
 * @file unroll.hpp
 *
 * @brief
 *     loop unroll optimization.
 *
 */

#pragma once

#include "opt/opt.hpp"

namespace minijava
{
	namespace opt
	{
		class unroll: public optimization
		{
		private:
			bool _changed{false};

		public:
			virtual bool optimize(firm_ir &) override;
		};
	}
}