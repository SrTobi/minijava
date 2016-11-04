/**
 * @file global.hpp
 *
 * @brief
 *     This header file provides global definitions for the project.
 *
 * If we ever decide to use pre-compiled headers, this is the candidate.
 *
 */

#pragma once

#include "config.h"


#ifdef NDEBUG
#   define MINIJAVA_ASSERT_ACTIVE 0
#else
#   define MINIJAVA_ASSERT_ACTIVE 1
#endif

/**
 * @brief
 *     Top-level `namespace` for everything in this project.
 *
 */
namespace minijava
{
	// We open this namespace here just so we can put a DocString on it.  This
	// file seems like a good place for this.

}
