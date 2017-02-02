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

			struct loop_info {
				firm::ir_loop *loop;
				size_t node_count{0};
				size_t call_count{0};
				size_t phi_count{0};
			};
		};
	}
}