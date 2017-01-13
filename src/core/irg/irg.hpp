/**
 * @file irg.hpp
 *
 * @brief
 *     Public interface for the IR graph generation.
 *
 */

#pragma once

#include <string>

#include "irg/global_firm_state.hpp"
#include "io/file_output.hpp"
#include "parser/ast.hpp"
#include "semantic/semantic.hpp"
#include "firm.hpp"

namespace minijava
{

	/**
	 * @brief
	 *     Initializes `libfirm`.
	 *
	 * @returns
	 *     global Firm state
	 *
	 * @throws std::logic_error
	 *     if `libfirm` was already initialized before
	 *
	 */
	std::unique_ptr<global_firm_state> initialize_firm();

	/** @brief References a IRG of a program and its global `libfirm` state. */
	using firm_ir_entry = std::pair<firm::ir_prog*, global_firm_state*>;

	/** @brief RAII handle for a program IRG. */
	using firm_ir = std::unique_ptr<firm_ir_entry, void(*)(firm_ir_entry*)>;

	/**
	 * @brief
	 *     Sets the global IRG pointer in `libfirm`.
	 *
	 * @param state
	 *     global `libfirm` state
	 *
	 * @param prog
	 *     IRG to operate on
	 *
	 * @returns
	 *     RAII guard that will reset the global IRG again
	 *
	 */
	std::unique_ptr<firm::ir_prog, void(*)(firm::ir_prog*)>
	make_irp_guard(const global_firm_state& state, firm::ir_prog*const prog);

	/**
	 * @brief
	 *     Creates an intermediate representation of the given program.
	 *
	 * This function may only be called after semantic analysis of the given
	 * program has completed successfully. If `semantic_info` does not contain
	 * all information produced during the semantic analysis of `ast`, the
	 * behavior is undefined.
	 *
	 * @param state
	 *     the global state of libfirm
	 *
	 * @param ast
	 *     program
	 *
	 * @param semantic_info
	 *     semantic information about the given program
	 *
	 * @param name
	 *     name of the IR graph to create
	 *
	 * @return
	 *     intermediate representation
	 *
	 * @throws std::logic_error
	 *     if `libfirm`'s global state is not what is expected
	 *
	 */
	firm_ir create_firm_ir(global_firm_state& state,
	                       const ast::program& ast,
	                       const semantic_info& semantic_info,
	                       const std::string& name);

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
	 * @throws std::logic_error
	 *     if `libfirm`'s global state is not what is expected
	 *
	 */
	void dump_firm_ir(firm_ir& ir, const std::string& directory = "");

	/**
	 * @brief
	 *     Converts the given intermediate representation into x64 assembly
	 *     using Firm's own backend and writes it to the given file.
	 *
	 * @param ir
	 *     intermediate representation
	 *
	 * @param output_file
	 *     assembly file
	 *
	 * @throws std::logic_error
	 *     if `libfirm`'s global state is not what is expected
	 *
	 */
	void emit_x64_assembly_firm(firm_ir& ir, file_output& output_file);

}  // namespace minijava
