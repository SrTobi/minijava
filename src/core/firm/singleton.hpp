/**
 * @file singleton.hpp
 *
 * @brief
 *     RAII guard for `libfirm`'s global state sadness.
 *
 */

#pragma once


#include <memory>

#include "parser/ast.hpp"
#include "semantic/attribute.hpp"


struct ir_node;


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
	 * At the time of writing (2016), this is a known limitation of libfirm.
	 *
	 */
	class firm_global_state final
	{

	public:

		/**
		 * @brief
		 *     Type of the argument list map, which retains the array of
		 *     pointers to IR nodes representing its arguments for each
		 *     `method_invocation` and memory allocation.
		 *
		 */
		using argument_list_map = ast_attributes<
				std::unique_ptr<ir_node*[]>,
				ast_node_filter<
						ast::method_invocation,
						ast::object_instantiation,
						ast::array_instantiation
				>
		>;

		/**
		 * @brief
		 *     Initializes libfirm.
		 *
		 * @throws std::logic_error
		 *     if libfirm was already initialized
		 *
		 */
		firm_global_state();

		/**
		 * @brief
		 *     `delete`d copy constructor.
		 *
		 * `firm_global_state` objects are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 */
		firm_global_state(const firm_global_state& other) = delete;

		/**
		 * @brief
		 *     `delete`d copy assignment operator.
		 *
		 * `firm_global_state` objects are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		firm_global_state& operator=(const firm_global_state& other) = delete;

		/**
		 * @brief
		 *     Move constructor.
		 *
		 * Creates a `firm_global_state` object which takes over the ownership of
		 * `libfirm` from `other`, which must not be used after calling this
		 * constructor.
		 *
		 * @param other
		 *     `firm_global_state` object to transfer libfirm ownership from
		 *
		 * @throws std::logic_error
		 *     if `other` was previously moved and does not own libfirm anymore
		 *
		 */
		firm_global_state(firm_global_state&& other);

		/**
		 * @brief
		 *     `delete`d copy assignment operator.
		 *
		 * Since there cannot be more than one default-constructed `firm_global_state`
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
		firm_global_state& operator=(firm_global_state&& other) = delete;

		/**
		 * @brief
		 *     Frees the dynamic memory allocated by libfirm.
		 *
		 * This does not fully reset libfirm's internal state due to a known
		 * limitation of libfirm.  Creating a new `firm_global_state` object
		 * afterwards is not possible, once the first `firm_global_state`
		 * object was destroyed.
		 *
		 */
		~firm_global_state();

		/**
		 * @brief
		 *     Returns the argument list map, which retains the array of
		 *     pointers to IR nodes representing its arguments for each
		 *     `method_invocation` and memory allocation.
		 *
		 * @return
		 *     arguments map
		 *
		 */
		argument_list_map& arguments_map()
		{
			return _arguments_map;
		}

		/**
		 * @brief
		 *     Tests whether this object actively owns the global `libfirm`
		 *     state.
		 *
		 * @returns
		 *     whether this object actively owns `libfirm`'s global state
		 *
		 */
		operator bool() const noexcept;

	private:

		/**
		 * @brief
		 *     Argument list map, which retains the array of pointers to IR
		 *     nodes representing its arguments for each `method_invocation`
		 *     and memory allocation.
		 *
		 * libfirm requires the user to allocate an array for each method
		 * invocation. Those arrays are saved in this data structure to prevent
		 * memory leaks.
		 *
		 */
		argument_list_map _arguments_map{};

		/**
		 * @brief
		 *     Whether this instance should deallocate libfirm's data structures
		 *     upon destruction.
		 *
		 */
		bool _firm_owner{true};

	};

}  // namespace minijava
