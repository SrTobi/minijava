/**
 * @file irg.hpp
 *
 * @brief
 *     Public interface for the IR graph generation.
 *
 */

#pragma once

#include <string>

#include "irg/singleton.hpp"
#include "io/file_output.hpp"
#include "parser/ast.hpp"
#include "semantic/semantic.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Creates an intermediate representation of the given program.
	 *
	 * This function may only be called after semantic analysis of the given
	 * program has completed successfully. If `semantic_info` does not contain
	 * all information produced during the semantic analysis of `ast`, the
	 * behavior is undefined.
	 *
	 * @param ast
	 *     program
	 *
	 * @param semantic_info
	 *     semantic information about the given program
	 *
	 * @return
	 *     intermediate representation
	 *
	 */
	firm_global_state create_firm_ir(const ast::program& ast, const semantic_info& semantic_info);

	/**
	 * @brief
	 *     Dumps graph files describing the given intermediate representation
	 *     into the given directory.
	 *
	 * @param ir
	 *     intermediate representation
	 *
	 * @param directory
	 *     target directory (default: current working directory)
	 *
	 */
	void dump_firm_ir(firm_global_state& ir, const std::string& directory = "");

	/**
	 * @brief
	 *     Converts the given intermediate representation into x64 assembly
	 *     using libfirm's backend and writes it to the given file.
	 *
	 * @param ir
	 *     intermediate representation
	 *
	 * @param output_file
	 *     assembly file
	 *
	 */
	void emit_x64_assembly_firm(firm_global_state& ir, file_output& output_file);

}  // namespace minijava
