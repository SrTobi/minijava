#include <firm/builder.hpp>

void minijava::ir_types::create_and_finalize_method_body(
		const minijava::ast::main_method &method,
		ir_graph* irg,
		ir_type* class_type)
{
	set_current_ir_graph(irg);

	// call method_builder
	firm::create_firm_method(_semantic_info, *this, *class_type, method);

	mature_immBlock(get_irg_end_block(irg));

	irg_finalize_cons(irg);
	irg_verify(irg);
}

void minijava::ir_types::create_and_finalize_method_body(
		const minijava::ast::instance_method &method,
		ir_graph* irg,
		ir_type* class_type)
{
	set_current_ir_graph(irg);

	// call method_builder
	firm::create_firm_method(_semantic_info, *this, *class_type, method);

	mature_immBlock(get_irg_end_block(irg));

	irg_finalize_cons(irg);
	dump_ir_graph(irg, "test");
	irg_verify(irg);
}

void minijava::ir_types::create_method_entity(
		ir_type *class_type,
        const std::unique_ptr<minijava::ast::instance_method> &method)
{
	(void)class_type;
	auto method_entity = get_method_entity(method.get());
	auto irg = new_ir_graph(method_entity, get_local_var_count(*method));

	create_and_finalize_method_body(*method.get(), irg, class_type);

//  default_layout_compound_type(class_type);
//  set_type_state(class_type, layout_fixed);
//  dump_ir_graph(irg, method->name().c_str());
}

void minijava::ir_types::create_method_entity(
		ir_type *class_type,
		const std::unique_ptr<minijava::ast::main_method> &method)
{
	(void)class_type;
	auto method_entity = get_method_entity(method.get());
	auto irg = new_ir_graph(method_entity, get_local_var_count(*method));

	create_and_finalize_method_body(*method, irg, class_type);

//  dump_ir_graph(irg, "test_x_");
//  dump_all_ir_graphs("");
}
