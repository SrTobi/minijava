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

			struct inline_info {
				firm::ir_tarval* tarval;
				int no_return_mem_count{0};
				int return_count{0};
				int phi_count{0};

				constexpr bool no_mem() { return no_return_mem_count == return_count; }
				constexpr bool has_phi() { return phi_count > 0; }
			};
		};
	}
}