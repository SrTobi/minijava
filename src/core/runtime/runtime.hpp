/**
 * @file runtime.hpp
 *
 * @brief
 *     Runtime support library access.
 *
 * The MiniJava runtime support library is provided as a single string of C
 * source code that should eb compiled and linked with each MiniJava program.
 *
 * It defines the following symbols:
 *
 * - `mj_runtime_allocate`
 * - `mj_runtime_println`
 * - `main`
 *
 * And it expects the following symbols to be provided:
 *
 * - `minijava_main`
 *
 * The functions have the following interface.
 *
 *     void* mj_runtime_allocate(size_t n, size_t sz);
 *
 * Allocates memory for `n` objects of size `sz` each.
 *
 *     void mj_runtime_println(int32_t n);
 *
 * Prints the decimal value of `n` followed by a newline character to standard
 * output.
 *
 *     int main(int argc, char** argv);
 *
 * Must not be called; but will call `minijava_main`.
 *
 *     void minijava_main(void);
 *
 * This function must be provided and should run the MiniJava program.
 *
 * The whole program must still be linked to the target's C standard library.
 *
 */

#pragma once

#include <string>


namespace minijava
{

	/**
	 * @brief
	 *     `return`s the C source code of the runtime support library as one
	 *     large string.
	 *
	 * @returns
	 *     C source code for runtime support library
	 *
	 */
	// TODO: Return a `boost::string_ref` one we finally switch to that.
	std::string runtime_source();

}  // namespace minijava
