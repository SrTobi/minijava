#include <firm/builder.hpp>

void minijava::ir_types::create_and_finalize_method_body(const minijava::ast::method &/*method*/, ir_graph* irg)
{
	set_current_ir_graph(irg);

	ir_node* start = get_irg_start(irg);
	ir_node* args = new_Proj(start, mode_T, pn_Start_T_args);
	ir_entity* m_ent = get_irg_entity(irg);
	ir_type* m_type = get_entity_type(m_ent);
	size_t param_count = get_method_n_params(m_type);

	for (size_t i = 0; i < param_count; i++) {
		auto p_type = get_method_param_type(m_type, i);
		ir_node* arg = new_Proj(args, get_type_mode(p_type), (unsigned int) i);
		set_value((int) i, arg);
	}

	irg_finalize_cons(irg);
	irg_verify(irg);
}

void minijava::ir_types::create_method_entity(
		ir_type *class_type,
        const std::unique_ptr<minijava::ast::instance_method> &method)
{
	auto method_entity = get_method_entity(method.get());
	auto irg = new_ir_graph(method_entity, get_local_var_count(*method));

	set_cur_block(get_irg_start_block(irg));
	create_and_finalize_method_body(*method.get(), irg);

	default_layout_compound_type(class_type);
	set_type_state(class_type, layout_fixed);
	dump_ir_graph(irg, method->name().c_str());
}

void minijava::ir_types::create_method_entity(
		ir_type *class_type,
		const std::unique_ptr<minijava::ast::main_method> &method)
{
	(void)class_type;
	auto method_entity = get_method_entity(method.get());
	auto count = get_local_var_count(*method);
	auto irg = new_ir_graph(method_entity, count);
	set_current_ir_graph(irg);


	auto store = get_store();
	auto ret = new_Return(store, 0, 0);
	add_immBlock_pred(get_irg_end_block(irg), ret);
	mature_immBlock(get_r_cur_block(irg));
	irg_finalize_cons(irg);

	dump_ir_graph(irg, "test_x_");
	dump_all_ir_graphs("");
}
