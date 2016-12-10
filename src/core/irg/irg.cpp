#include "irg/irg.hpp"

#include "firm.hpp"

#include "irg/builtins.hpp"
#include "irg/lowering.hpp"
#include "irg/method_builder.hpp"
#include "irg/type_builder.hpp"


namespace minijava
{
	namespace {
		void destroy_prog(firm::ir_prog* prog)
		{
			assert(prog);
			/* BUG:assert(!firm::get_irp()); */
			firm::set_irp(prog);
			firm::free_ir_prog();
		}

		void existing_irp_check()
		{
			if(firm::get_irp() && false /* BUG */) {
				throw std::logic_error(
					"Another program is currently under construction"
				);
			}
		}

		auto set_irp_temporarily(firm_ir& res)
		{
			firm::set_irp(res.get());
			auto deleter = [](firm::ir_prog* /*irp*/) { /* BUG(libfirm can not handle firm::new_ir_prog if this is empty): firm::set_irp(nullptr);*/ };
			return std::unique_ptr<firm::ir_prog, decltype(deleter)>{res.get(), deleter};
		}
	}

	firm_ir create_firm_ir(global_firm_state& state,
	                       const ast::program& ast,
	                       const semantic_info& semantic_info,
	                       const std::string& name)
	{
		assert(state);
		existing_irp_check();
		auto ir = firm_ir{firm::new_ir_prog(name.c_str()), &destroy_prog};
		const auto guard = set_irp_temporarily(ir);
		auto types = irg::create_types(ast, semantic_info);
		irg::create_globals(semantic_info, types);
		irg::create_methods(semantic_info, types);
		return ir;
	}

	// since libfirm keeps the implicit state, we don't actually need the first
	// parameter; it's just to make sure that the user keeps the firm_global_state object
	void dump_firm_ir(firm_ir& ir, const std::string& directory)
	{
		assert(ir);
		existing_irp_check();
		const auto guard = set_irp_temporarily(ir);
		if (!directory.empty()) {
			firm::ir_set_dump_path(directory.c_str());
		}
		firm::dump_all_ir_graphs("");
	}


	// since libfirm keeps the implicit state, we don't actually need the first
	// parameter; it's just to make sure that the user keeps the firm_ir object
	void emit_x64_assembly_firm(firm_ir& ir, file_output& output_file)
	{
		assert(ir);
		existing_irp_check();
		const auto guard = set_irp_temporarily(ir);
		irg::lower();
		firm::be_parse_arg("isa=amd64");
		firm::be_main(output_file.handle(), output_file.filename().c_str());
	}

}  // namespace minijava
