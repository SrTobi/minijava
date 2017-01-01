/**
 * @file opt.hpp
 *
 * @brief
 *     Interface for optimizations.
 *
 */

#pragma once

#include "irg/irg.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Namespace for optimizations.
	 */
	namespace opt
	{
		/**
		 * @brief
		 *     Base class all optimizations should inherit from and implement
		 */
		class optimization
		{
		public:
			/**
			 * @brief
			 *     Optimizes the given Firm IRG.
			 *     Should be overwritten.
			 * @param ir
			 *     IRG to optimize
			 * @return
			 *     Returns true, if something has changed in the IRG, otherwhise false
			 */
			virtual bool optimize(firm_ir& /*ir*/) = 0;
		};
	}

	/**
	 * @brief
	 *     Optimizes the given Firm IRG.
	 *
	 * @param ir
	 *     IRG to optimize
	 *
	 */
	void optimize(firm_ir& ir);

	/**
	 * @brief
	 *     Registers all known optimizatios to be evaluated before running the backend
	 */
	void register_all_optimizations();

	/**
	 * @brief
	 *     Registers a single optimization to be evaluated before running the backend
	 * @param opt
	 *     Optimization to be evaluated
	 */
	void register_optimization(std::unique_ptr<minijava::opt::optimization> opt);

}  // namespace minijava
