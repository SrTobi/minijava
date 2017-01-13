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
	 *     Optimizes the given Firm IRG.
	 *
	 * @param ir
	 *     IRG to optimize
	 *
	 */
	void optimize(firm_ir& ir);

}  // namespace minijava
