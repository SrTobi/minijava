#include "irg/irg.hpp"

#include <stdexcept>

#include "firm.hpp"
#include "irg/globals.hpp"
#include "irg/method_builder.hpp"
#include "irg/type_builder.hpp"


namespace minijava
{

	std::unique_ptr<global_firm_state> initialize_firm()
	{
		return std::make_unique<global_firm_state>();
	}

	namespace /* anonymous */
	{

		auto destroy(firm_ir_entry*const p)
		{
			// If `global_firm_state::program_count()` is not zero, it is safe
			// to dereference the pointer in `p->second`.
			assert(p != nullptr);
			assert(global_firm_state::program_count());
			if (p->first != nullptr) {
				assert(p->second != nullptr);
				p->second->free_ir_prog(p->first);
			}
			delete p;
		}

		std::unique_ptr<firm::ir_prog, void(*)(firm::ir_prog*)>
		make_irp_guard(const global_firm_state& state)
		{
			assert(global_firm_state::program_count());
			const auto old = firm::get_irp();
			if (old != state.get_default_irp()) {
				throw std::logic_error{"Somebody is messing with libfirm's global state"};
			}
			auto del = [](firm::ir_prog* p) { firm::set_irp(p); };
			return std::unique_ptr<firm::ir_prog, void(*)(firm::ir_prog*)>{old, del};
		}

	}  // namespace /* anonymous */

	std::unique_ptr<firm::ir_prog, void(*)(firm::ir_prog*)>
	make_irp_guard(const global_firm_state& state, firm::ir_prog*const p)
	{
		auto guard = make_irp_guard(state);
		firm::set_irp(p);
		return guard;
	}

	firm_ir create_firm_ir(global_firm_state& state,
	                       const ast::program& ast,
	                       const semantic_info& semantic_info,
	                       const std::string& name)
	{
		const auto guard = make_irp_guard(state);
		auto ir = firm_ir{new firm_ir_entry{nullptr, nullptr}, &destroy};
		ir->second = &state;
		ir->first = state.new_ir_prog(name.c_str());
		auto types = irg::create_types(ast, semantic_info);
		irg::create_globals(semantic_info, types);
		irg::create_methods(semantic_info, types);
		return ir;
	}

	void dump_firm_ir(firm_ir& ir, const std::string& directory)
	{
		assert(ir);
		const auto guard = make_irp_guard(*ir->second, ir->first);
		if (!directory.empty()) {
			firm::ir_set_dump_path(directory.c_str());
		}
		firm::dump_all_ir_graphs("");
	}

}  // namespace minijava
