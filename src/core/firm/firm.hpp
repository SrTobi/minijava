/**
 * @file firm.hpp
 *
 * @brief
 *     Public interface for the IR graph generation.
 *
 */

#pragma once

#include <string>

#include "io/file_output.hpp"
#include "parser/ast.hpp"
#include "semantic/semantic.hpp"

namespace minijava
{

	/**
	 * @brief
	 *     RAII wrapper around the intermediate representation created by
	 *     libfirm.
	 *
	 * Due to libfirm's internal state keeping, users must not create more than
	 * one instance of this class during the entire lifetime of a program,
	 * unless they use the move constructor to transfer ownership of libfirm
	 * from the previous instance.
	 *
	 * (At the time of writing (2016), this is a known limitation of libfirm.)
	 *
	 */
	class firm_ir final
	{

	public:

		/**
		 * @brief
		 *     Initializes libfirm.
		 *
		 * @throws std::logic_error
		 *     if libfirm was already initialized
		 *
		 */
		firm_ir();

		/**
		 * @brief
		 *     `delete`d copy constructor.
		 *
		 * `firm_ir` objects are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 */
		firm_ir(const firm_ir& other) = delete;

		/**
		 * @brief
		 *     `delete`d copy assignment operator.
		 *
		 * `firm_ir` objects are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		firm_ir& operator=(const firm_ir& other) = delete;

		/**
		 * @brief
		 *     Move constructor.
		 *
		 * Creates a `firm_ir` object which takes over the ownership of
		 * `libfirm` from `other`, which must not be used after calling this
		 * constructor.
		 *
		 * @param other
		 *     `firm_ir` object to transfer libfirm ownership from
		 *
		 * @throws std::logic_error
		 *     if `other` was previously moved and does not own libfirm anymore
		 *
		 */
		firm_ir(firm_ir&& other);

		/**
		 * @brief
		 *     `delete`d copy assignment operator.
		 *
		 * Since there cannot be more than one default-constructed `firm_ir`
		 * instance in any given program, move assignment is not a useful
		 * operation.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		firm_ir& operator=(firm_ir&& other) = delete;

		/**
		 * @brief
		 *     Frees the dynamic memory allocated by libfirm.
		 *
		 * This does not fully reset libfirm's internal state due to a known
		 * limitation of libfirm.
		 *
		 */
		~firm_ir();

	private:

		/**
		 * @brief
		 *     whether this instance should deallocate libfirm's data structures
		 *     upon destruction
		 *
		 */
		bool _firm_owner{true};

	};

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
	firm_ir create_firm_ir(const ast::program& ast,
	                       const semantic_info& semantic_info);

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
	void dump_firm_ir(const firm_ir& ir, const std::string& directory = "");

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
	void emit_x64_assembly_firm(const firm_ir& ir, file_output& output_file);

}  // namespace minijava
