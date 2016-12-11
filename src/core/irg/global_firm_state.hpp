/**
 * @file global_firm_state.hpp
 *
 * @brief
 *     RAII guard for `libfirm`'s global state sadness.
 *
 */

#pragma once

#include "firm.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Representation of the global `libfirm` state.
	 *
	 * Normal code should not have a need to use any of the non-`static` member
	 * functions and merely use the object as an RAII guard.  The provided
	 * member functions are low-level tools for implementing higher level
	 * abstractions provided by `irg.hpp`.
	 *
	 */
	class global_firm_state final
	{
	public:

		/**
		 * @brief
		 *     Initializes `libfirm`.
		 *
		 * Due to a known limitation of `libfirm`, this function can only be
		 * called at most once in a program.  If it is called a second time an
		 * exception is `throw`n.  Note that it is also not possible to call
		 * this function again even after the previous `global_firm_state` has
		 * been destroyed, for doing so puts `libfirm` into a zombie state
		 * where it cannot be used at all any more.
		 *
		 * After initializing `libfirm`, `program_count` will report 1 program
		 * which is the default program that is implicitly created by
		 * `libfirm`.  It may be used but must never be freed.  Object-oriented
		 * code should probably leave this ainstance alone, though, and alway
		 * create and destroy its own as needed.
		 *
		 * @throws std::logic_error
		 *     if `libfirm` was already initialized before
		 *
		 */
		global_firm_state();

		/**
		 * @brief
		 *     Deinitializes `libfirm` as far as this is possible.
		 *
		 * After deinitialization, `libfirm` is in a zombie state and must not
		 * be used at all.
		 *
		 * At the time where this destructor runs, the only live program must
		 * be the implicitly created program.  Otherwise, the behavior is
		 * undefined.
		 *
		 */
		~global_firm_state();

		/**
		 * @brief
		 *     `delete`d copy constructor.
		 *
		 * @param other
		 *     *N/A*
		 *
		 */
		global_firm_state(const global_firm_state& other) = delete;

		/**
		 * @brief
		 *     `delete`d copy assignment-operator.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		global_firm_state& operator=(const global_firm_state& other) = delete;

		/**
		 * @brief
		 *     Calls `new_ir_prog` from `libfirm` with the given argument and
		 *     increments the program count; then calls `set_irp` to make the
		 *     new program the current one.
		 *
		 * If `libfirm` is not properly initialized, the behavior is undefined.
		 *
		 * This is a low-level function that should only be used to implement
		 * proper RAII wrappers.
		 *
		 * @param name
		 *     argument to forward to `new_ir_prog`
		 *
		 * @returns
		 *     handle to the newly created program
		 *
		 */
		firm::ir_prog* new_ir_prog(const char* name);

		/**
		 * @brief
		 *     Calls `set_irp` from `libfirm` with the given argument followed
		 *     by `free_ir_prog` and finally `set_irp` with the default IRP;
		 *     then decrements the program count.
		 *
		 * If `libfirm` is not properly initialized, the behavior is undefined.
		 *
		 * This is a low-level function that should only be used to implement
		 * proper RAII wrappers.
		 *
		 * @param prog
		 *     program to destroy
		 *
		 */
		void free_ir_prog(firm::ir_prog* prog);

		/**
		 * @brief
		 *     `return`s the default program that is implicitly created by
		 *     `libfirm`.
		 *
		 * The only sane way to use this program is to make sure it is never
		 * used.
		 *
		 * @returns
		 *     implicitly created default program
		 *
		 */
		firm::ir_prog* get_default_irp() const noexcept;

		/**
		 * @brief
		 *     `return`s the number of currently live program instances in
		 *     Firm.
		 *
		 * This function may also be called before `libfirm` was initialized or
		 * after it was deinitialized.  In this case (and only in this case),
		 * it will `return` 0.
		 *
		 * Note that while `libfirm` is initialized, there is always at least
		 * one program.
		 *
		 * @returns
		 *     the current number of live program IRG instances
		 *
		 */
		static int program_count() noexcept;

	private:

		/** @brief The implcitly created global program. */
		firm::ir_prog* _irp{};

	};

}  // namespace minijava
