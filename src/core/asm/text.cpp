#include "asm/text.hpp"

#include <cassert>

#include <boost/variant/apply_visitor.hpp>

#include "exceptions.hpp"


namespace minijava
{

	namespace backend
	{

		void assemble_function(firm::ir_graph* irg, virtual_assembly& /*virtasm*/)
		{
			assert(irg != nullptr);
			(void) irg;
			MINIJAVA_NOT_IMPLEMENTED();
		}

		void allocate_registers(const virtual_assembly& /*virtasm*/, real_assembly& /*realasm*/)
		{
			MINIJAVA_NOT_IMPLEMENTED();
		}

		namespace /* anonymous */
		{

			// Returns the mnemotic of an opcode together with its width suffix
			// (if applicable) or the empty string if `code == opcode::none`.
			std::string format(const opcode code, const bit_width width)
			{
				auto formatted = std::string{};
				if (const auto base = mnemotic(code)) {
					formatted = base;
					switch (width) {
					case bit_width::viii:
						formatted.push_back('b');
						break;
					case bit_width::xvi:
						formatted.push_back('w');
						break;
					case bit_width::xxxii:
						formatted.push_back('l');
						break;
					case bit_width::lxiv:
						formatted.push_back('q');
						break;
					}
				}
				return formatted;
			}

			std::string format(const address<real_register>& addr, const bit_width width)
			{
				char buffer[100];
				const auto c = [&](){ return static_cast<long>(addr.constant.get()); };
				const auto b = [&](){ return name(addr.base.get(), width); };
				const auto i = [&](){ return name(addr.index.get(), width); };
				const auto s = [&](){ return static_cast<long>(addr.scale.get()); };
				const auto fmt = [&](const auto... args){
					const auto stat = std::snprintf(buffer, sizeof(buffer), args...);
					if (stat < 0) { MINIJAVA_THROW_ICE(internal_compiler_error); };
					return buffer;
				};
				const auto bits = 0U
					| ((addr.constant ? 1U : 0U) << 3)
					| ((addr.base     ? 1U : 0U) << 2)
					| ((addr.index    ? 1U : 0U) << 1)
					| ((addr.scale    ? 1U : 0U) << 0);
				switch (bits) {
				case 0b0010U: return fmt("(, %%%s)", i());
				case 0b0011U: return fmt("(, %%%s, %ld)", i(), s());
				case 0b0100U: return fmt("(%%%s)", b());
				case 0b0110U: return fmt("(%%%s, %%%s)", b(), i());
				case 0b0111U: return fmt("(%%%s, %%%s, %ld)", b(), i(), s());
				case 0b1000U: return fmt("%ld", c());
				case 0b1010U: return fmt("%ld(, %%%s)", c(), i());
				case 0b1011U: return fmt("%ld(, %%%s, %ld)", c(), i(), s());
				case 0b1100U: return fmt("%ld(%%%s)", c(), b());
				case 0b1110U: return fmt("%ld(%%%s, %%%s)", c(), b(), i());
				case 0b1111U: return fmt("%ld(%%%s, %%%s, %ld)", c(), b(), i(), s());
				default: MINIJAVA_NOT_REACHED();
				}
			}

			// Returns the AT&T representation of an operand or the empty
			// string if `empty(op)`.
			std::string format(const operand<real_register>& op, const bit_width width)
			{
				using namespace std::string_literals;
				using real_address = address<real_register>;
				struct visitor : boost::static_visitor<std::string>
				{
					bit_width width;

					visitor(const bit_width bw) : width{bw} {}

					std::string operator()(const boost::blank) const
					{
						return "";
					}

					std::string operator()(const std::int64_t imm) const
					{
						return "$" + std::to_string(imm);
					}

					std::string operator()(const real_register reg) const
					{
						return "%"s + name(reg, this->width);
					}

					std::string operator()(const real_address& addr) const
					{
						return format(addr, this->width);
					}

					std::string operator()(const boost::string_ref name) const
					{
						return std::string{name.data(), name.size()};
					}

				};
				return boost::apply_visitor(visitor{width}, op);
			}

		}  // namespace /* anonymous */

		void write_text(const real_assembly& realasm, file_output& out)
		{
			for (const auto& instr : realasm) {
				if (!instr.label.empty()) {
					out.write(instr.label.data(), instr.label.size());
					out.write(":\n");
				}
				const auto mnemotic = format(instr.code, instr.width);
				if (mnemotic.empty()) {
					continue;
				}
				const auto op1 = format(instr.op1, instr.width);
				const auto op2 = format(instr.op2, instr.width);
				const auto arity = 0 + !op2.empty() + !op1.empty();
				switch (arity) {
				case 0:
					out.print("\t%s\n", mnemotic.c_str());
					break;
				case 1:
					out.print("\t%s %s\n", mnemotic.c_str(), op1.c_str());
					break;
				case 2:
					out.print("\t%s %s, %s\n", mnemotic.c_str(), op1.c_str(), op2.c_str());
					break;
				default:
					MINIJAVA_NOT_REACHED();
				}
			}
		}

	}  // namespace backend

}  // namespace minijava
