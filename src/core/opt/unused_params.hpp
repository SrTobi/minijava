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
		private:
			bool _changed{false};
			void remove_unused_params(firm::ir_entity* method, std::vector<unsigned int> params_to_keep);

		public:
			virtual bool optimize(firm_ir &ir) override;
		};
	}
}