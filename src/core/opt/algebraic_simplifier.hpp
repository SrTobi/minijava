/**
 * @file algebraic_simplifier.hpp
 *
 * @brief
 *     Algebraic simplifier optimization.
 *
 */

#pragma once

#include "opt/opt.hpp"

namespace minijava
{
	namespace opt
	{
		class algebraic_simplifier : public optimization
		{
		public:

			virtual bool optimize(firm_ir& ir) override;



		private:
			bool _changed{};
			firm::ir_graph* _irg{};

			static void algebraic_walker(firm::ir_node* node, void* env);
			static void exchange_walker(firm::ir_node* node, void* env);
		};
	}
}