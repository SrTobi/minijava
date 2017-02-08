#include "asm/output.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>

#include <boost/utility/string_ref.hpp>
#include <boost/variant/apply_visitor.hpp>

#include "exceptions.hpp"


namespace minijava
{

	namespace backend
	{

		namespace /* anonymous */
		{

			std::string name(const virtual_register reg, const bit_width width)
			{
				auto regname = std::string{};
				switch (reg) {
				case virtual_register::dummy:
					regname = "Dummy";
					break;
				case virtual_register::result:
					regname = "RES";
					break;
				default:
				{
					const auto num = static_cast<int>(reg);
					regname = (num < 0)
						? "A" + std::to_string(-num)
						: "R" + std::to_string(num);
				}
				}
				regname += "_";
				regname += std::to_string(static_cast<int>(width));
				return regname;
			}

			const char* c_str(const char* s) noexcept { return s; }
			const char* c_str(const std::string& s) noexcept { return s.c_str(); }

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

			template <typename RegT>
			std::string format(const address<RegT>& addr, const bit_width width)
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
				case 0b0010U: return fmt("(, %%%s)", c_str(i()));
				case 0b0011U: return fmt("(, %%%s, %ld)", c_str(i()), s());
				case 0b0100U: return fmt("(%%%s)", c_str(b()));
				case 0b0110U: return fmt("(%%%s, %%%s)", c_str(b()), c_str(i()));
				case 0b0111U: return fmt("(%%%s, %%%s, %ld)", c_str(b()), c_str(i()), s());
				case 0b1000U: return fmt("%ld", c());
				case 0b1010U: return fmt("%ld(, %%%s)", c(), c_str(i()));
				case 0b1011U: return fmt("%ld(, %%%s, %ld)", c(), c_str(i()), s());
				case 0b1100U: return fmt("%ld(%%%s)", c(), c_str(b()));
				case 0b1110U: return fmt("%ld(%%%s, %%%s)", c(), c_str(b()), c_str(i()));
				case 0b1111U: return fmt("%ld(%%%s, %%%s, %ld)", c(), c_str(b()), c_str(i()), s());
				default: MINIJAVA_NOT_REACHED();
				}
			}

			// Returns the AT&T representation of an operand or the empty
			// string if `empty(op)`.
			template <typename RegT>
			std::string format(const operand<RegT>& op, const bit_width width)
			{
				using namespace std::string_literals;
				using address_type = address<RegT>;
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

					std::string operator()(const RegT reg) const
					{
						return "%"s + name(reg, this->width);
					}

					std::string operator()(const address_type& addr) const
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

			template <typename RegT>
			std::pair<bit_width, bit_width> get_operand_widths(const instruction<RegT>& instr)
			{
				switch (instr.code) {
				case opcode::op_movslq:
					assert(instr.width == bit_width{});
					return std::make_pair(bit_width::xxxii, bit_width::lxiv);
				case opcode::op_lea:
					assert(instr.width == bit_width{});
					return std::make_pair(bit_width::lxiv, bit_width::lxiv);
				case opcode::op_seta:
				case opcode::op_setae:
				case opcode::op_setb:
				case opcode::op_setbe:
				case opcode::op_sete:
				case opcode::op_setne:
					return std::make_pair(bit_width::viii, bit_width{});
					// TODO: Add other cases here even though we don't need them?
				default:
					return std::make_pair(instr.width, instr.width);
				}
			}

			void write_label(const boost::string_ref label, file_output& out)
			{
				// Cannot use `print()` because `boost::string_ref` is not NUL
				// terminated.
				if (!label.empty()) {
					out.write(label.data(), label.size());
					out.write(":\n");
				}
			}

			template <typename RegT>
			void write_text_impl(const assembly<RegT>& assembly, file_output& out)
			{
				write_label(assembly.ldname, out);
				for (const auto& bb : assembly.blocks) {
					write_label(bb.label, out);
					for (const auto& instr : bb.code) {
						const auto mnemotic = format(instr.code, instr.width);
						if (mnemotic.empty()) {
							continue;
						}
						const auto width = get_operand_widths(instr);
						const auto op1 = format(instr.op1, width.first);
						const auto op2 = format(instr.op2, width.second);
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
			}

		}  // namespace /* anonymous */

		void write_text(const virtual_assembly& asmcode, file_output& out)
		{
			write_text_impl(asmcode, out);
		}

		void write_text(const real_assembly& asmcode, file_output& out)
		{
			write_text_impl(asmcode, out);
		}

	}  // namespace backend

}  // namespace minijava
