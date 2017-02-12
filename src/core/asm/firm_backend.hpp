/**
 * @file firm_backend.hpp
 *
 * @brief
 *     Assembly code generation backend from `libfirm`.
 *
 */

#pragma once

#include "io/file_output.hpp"
#include "irg/irg.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Converts the given intermediate representation into x64 assembly
	 *     using Firm's own backend and writes it to the given file.
	 *
	 * @param ir
	 *     intermediate representation after lowering
	 *
	 * @param output_file
	 *     assembly file
	 *
	 * @throws std::logic_error
	 *     if `libfirm`'s global state is not what is expected
	 *
	 */
	void emit_x64_assembly_firm(firm_ir& ir, file_output& output_file);

}
