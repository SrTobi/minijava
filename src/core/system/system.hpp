/**
 * @file system.hpp
 *
 * @brief
 *     Platform specific features wrapped in functions that can be used by
 *     platform-agnostic code.
 *
 */

#pragma once

#include <cstddef>


namespace minijava
{

	/**
	 * @brief
	 *     Sets the operating system limit for the maximum allowed stack size.
	 *
	 * If `limit` is zero, only the current limit is `return`ed and no changes
	 * are performed.
	 *
	 * If `limit` is negative, the limit is set to the maximum value allowed by
	 * the operating system for this process.
	 *
	 * On platforms where the `setrlimit` system call is not available, this
	 * function has no effect and always `throw`s a `std::system_error` with an
	 * error code of `ENOSYS`.  The value of the global `errno` variable is
	 * ignored on entry and unspecified on exit.
	 *
	 * @param limit
	 *     desired limit for the maximum stack size in bytes
	 *
	 * @returns
	 *     the previous value of the limit
	 *
	 * @throws std::system_error
	 *     if the limit could not be set as requested
	 *
	 */
	std::ptrdiff_t set_max_stack_size_limit(std::ptrdiff_t limit);

}  // namespace minijava
