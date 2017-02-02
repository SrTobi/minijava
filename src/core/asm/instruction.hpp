/**
 * @file instruction.hpp
 *
 * @brief
 *     x64 assembly instructions.
 *
 */

#pragma once

#include <cstdint>
#include <vector>

#include <boost/optional.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/variant.hpp>

#include "asm/opcode.hpp"
#include "asm/register.hpp"


namespace minijava
{

	namespace backend
	{

		/**
		 * @brief
		 *     An x64 address.
		 *
		 * This type is simply an aggregate of the possible components of an
		 * address.  A default-constructed address is not valid.
		 *
		 */
		template <typename RegT>
		struct address final
		{

			/** @brief Constant offset. */
			boost::optional<std::int32_t> constant{};

			/** @brief Base register. */
			boost::optional<RegT> base{};

			/** @brief Index register (must not be SP). */
			boost::optional<RegT> index{};

			/** @brief Element size (must be 1, 2, 4, or 8). */
			boost::optional<std::int8_t> scale{};

		};


		/**
		 * @brief
		 *     Discriminated union for possible operand types.
		 *
		 * An operand may be of the following types.
		 *
		 *  - `boost::blank` indicates that there is no operand
		 *  - `std::int64_t` is used for immediates / constants
		 *    (accessible via `get_immediate`)
		 *  - `RegT` is used for registers
		 *    (accessible via `get_register`)
		 *  - `address` is used for addresses
		 *    (accessible via `get_address`)
		 *  - `boost::string_ref` is used for names (labels)
		 *    (accessible via `get_name`)
		 *
		 * Since these types are not self-explanatory, instead of accessing
		 * them directly, the use of the `get_*` functions is recommended.
		 *
		 * The `operand` only stores a `boost::string_ref` and dones't actually
		 * own the string data of a name.  It is the user's responsibility to
		 * keep the referenced data valid as long as it is needed.
		 *
		 * @tparam RegT
		 *     register type (virtual or real)
		 *
		 */
		template <typename RegT>
		using operand = boost::variant<boost::blank, std::int64_t, RegT, address<RegT>, boost::string_ref>;

		/**
		 * @brief
		 *     Tests whether an operand is empty (absent).
		 *
		 * @tparam RegT
		 *     register type (virtual or real)
		 *
		 * @param op
		 *     operand
		 *
		 * @returns
		 *     `true` if the operand is empty
		 *
		 */
		template <typename RegT>
		bool empty(const operand<RegT>& op) noexcept
		{
			return boost::get<boost::blank>(&op);
		}

		/**
		 * @brief
		 *     `return`s a pointer to the immediate value of an operand or
		 *     `nullptr` if the operand is not an immediate.
		 *
		 * @tparam RegT
		 *     register type (virtual or real)
		 *
		 * @param op
		 *     operand
		 *
		 * @returns
		 *     pointer to immediate
		 *
		 */
		template <typename RegT>
		const std::int64_t* get_immediate(const operand<RegT>& op) noexcept
		{
			return boost::get<std::int64_t>(&op);
		}

		/**
		 * @brief
		 *     `return`s a pointer to the register value of an operand or
		 *     `nullptr` if the operand is not a register.
		 *
		 * @tparam RegT
		 *     register type (virtual or real)
		 *
		 * @param op
		 *     operand
		 *
		 * @returns
		 *     pointer to register
		 *
		 */
		template <typename RegT>
		const RegT* get_register(const operand<RegT>& op) noexcept
		{
			return boost::get<RegT>(&op);
		}

		/**
		 * @brief
		 *     `return`s a pointer to the address value of an operand or
		 *     `nullptr` if the operand is not an address.
		 *
		 * @tparam RegT
		 *     register type (virtual or real)
		 *
		 * @param op
		 *     operand
		 *
		 * @returns
		 *     pointer to address
		 *
		 */
		template <typename RegT>
		const address<RegT>* get_address(const operand<RegT>& op) noexcept
		{
			return boost::get<address<RegT>>(&op);
		}

		/**
		 * @brief
		 *     `return`s a pointer to the name value of an operand or
		 *     `nullptr` if the operand is not a name.
		 *
		 * @tparam RegT
		 *     register type (virtual or real)
		 *
		 * @param op
		 *     operand
		 *
		 * @returns
		 *     pointer to name
		 *
		 */
		template <typename RegT>
		const boost::string_ref* get_name(const operand<RegT>& op) noexcept
		{
			return boost::get<boost::string_ref>(&op);
		}

		/**
		 * @brief
		 *     An x64 instruction with a width, operands and an optional label.
		 *
		 * This type is simply an aggregate holding all information about the
		 * instruction.  A default-constructed `instruction` is empty (which is
		 * a valid state).  There is currently no validation built into this
		 * type because there are too many instructions with too many rules.
		 *
		 * @tparam RegT
		 *     register type (virtual or real)
		 *
		 */
		template <typename RegT>
		struct instruction final
		{

			/** @brief Type of operands. */
			using operand_type = operand<RegT>;

			/**
			 * @brief
			 *     Creates an instruction from the given arguments.
			 *
			 * All parameters are optional and default.
			 *
			 * @param code
			 *     opcode / mnemotic
			 *
			 * @param width
			 *     width (if applicable)
			 *
			 * @param op1
			 *     first (source) operand
			 *
			 * @param op2
			 *     second (destination) operand
			 *
			 * @param label
			 *     label
			 *
			 */
			instruction(const opcode code = opcode{},
						const bit_width width = bit_width{},
						operand_type op1 = operand_type{},
						operand_type op2 = operand_type{},
						boost::string_ref label = boost::string_ref{})
				: code{code}
				, width{width}
				, op1{std::move(op1)}
				, op2{std::move(op2)}
				, label{std::move(label)}
			{
			}

			/** @brief Opcode / mnemotic of the instruction. */
			opcode code{};

			/** @brief Width of the instruction (if applicable). */
			bit_width width{};

			/** @brief First (source) operand of the instruction. */
			operand<RegT> op1{};

			/** @brief Second (destination) operand of the instruction. */
			operand<RegT> op2{};

			/** @brief Optional label of the instruction. */
			boost::string_ref label{};

		};

		/** @brief Type for x64 instructions using virtual registers. */
		using virtual_instruction = instruction<virtual_register>;

		/** @brief Type for x64 instructions using real registers. */
		using real_instruction = instruction<real_register>;

		/** @brief Type used for x64 assemblies using virtual registers. */
		using virtual_assembly = std::vector<virtual_instruction>;

		/** @brief Type used for x64 assemblies using real registers. */
		using real_assembly = std::vector<real_instruction>;

	}  // namespace backend

}  // namespace minijava
