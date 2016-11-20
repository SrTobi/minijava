/**
 * @file misc.hpp
 *
 * @brief
 *     Miscellaneous utility functions for dealing with I/O.
 *
 * The stuff provided by this head is mostly low-level and probably of no use
 * outside implementations of I/O utilities.
 *
 */

#pragma once

#include <string>


namespace minijava
{

	/**
	 * @brief
	 *     Performs some very basic sanity checks on a string that is supposed
	 *     to be a file-name.
	 *
	 * This function is not supposed to be a fully-fledged validator nor even a
	 * basic sanity check.  Its sole purpose is to make sure that a string can
	 * safely be passed as file-name to C functions.  That is, it is not empty
	 * and contains no embedded NUL bytes.  Everything else is the job of the
	 * operating system.
	 *
	 * If the check fails, a `std::system_error` with an error code of `EINVAL`
	 * is `throw`n.  Otherwise, this function has no effect.
	 *
	 * @param filename
	 *     string to check
	 *
	 * @throws std::system_error
	 *     if the string is certainly not a valid file-name
	 *
	 */
	void check_file_name(const std::string& filename);

}  // namespace minijava
