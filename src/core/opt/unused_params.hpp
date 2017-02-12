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
		/**
		 * @brief
		 *     Removes unused params from all graphs in the given irp.
		 *     Might be kind of useless, if we use the inline optimization.
		 */
		class unused_params : public optimization
		{
		private:
			/**
			 * Stores, if the optimization found and changed something
			 */
			bool _changed{false};

			/**
			 * @brief
			 *     Removes all params, not in the `params_to_keep` list and
			 *     update all call nodes to the given method entity.
			 *     To achieve this, a new method entity is created.
			 * @param method
			 * @param params_to_keep
			 */
			void remove_unused_params(firm::ir_entity* method, std::vector<unsigned int> params_to_keep);

		public:

			/**
			 * @brief
			 *     Runs the optimization on all graphs in the given irp
			 * @param ir
			 * @return
			 */
			virtual bool optimize(firm_ir &ir) override;
		};
	}
}
