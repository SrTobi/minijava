/**
 * @file global.hpp
 *
 * @brief
 *     This header file provides global definitions for the project.
 *
 * If we ever decide to use pre-compiled headers, this is the candidate.
 *
 * This file also <code>\#define</code>s macros for all environment variables
 * that are used by the program.  This allows us to use a single definition
 * point and let the compiler catch potential misspellings.  The naming
 * convention is to use a macro `MINIJAVA_ENVVAR_FOO` for the environment
 * variable `MINIJAVA_FOO`.
 *
 */

#pragma once

#include "config.h"


#ifdef NDEBUG
#   define MINIJAVA_ASSERT_ACTIVE 0
#else
/**
 * @brief
 *     Controls whether code paths that check additional assertions are enabled.
 *
 */
#   define MINIJAVA_ASSERT_ACTIVE 1
#endif

/**
 * @brief
 *     Environment variable that can be used to set the maximum stack size.
 *
 */
#define MINIJAVA_ENVVAR_STACK_LIMIT "MINIJAVA_STACK_LIMIT"


/**
 * @brief
 *     Environment variable that can be used to disable memory-mapped I/O.
 *
 */
#define MINIJAVA_ENVVAR_NO_MMAP "MINIJAVA_NO_MMAP"


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
