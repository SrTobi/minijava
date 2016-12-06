#include "firm/method_builder.hpp"

#include <set>
#include <unordered_map>

#include "libfirm/firm.h"

#include "exceptions.hpp"
#include "firm/builder.hpp"

namespace minijava
{

	namespace firm
	{

		namespace /* anonymous */
		{

			using var_id_map_type = std::unordered_map<const ast::var_decl*, int>;

			class expression_generator final : public ast::visitor
			{

			public:

				expression_generator(const semantic_info& sem_info,
				                     const var_id_map_type& var_ids,
				                     ir_types& firm_types, const ir_type* class_type)
						: _sem_info{sem_info}, _var_ids{var_ids},
						  _firm_types{firm_types}, _class_type{class_type}
				{}

				using ast::visitor::visit;

				void visit(const ast::boolean_constant &node) override
				{
					_current_node = new_Const_long(_firm_types.mode_boolean(), node.value());
				}

				void visit(const ast::integer_constant &node) override
				{
					auto value = _sem_info.const_annotations().at(node);
					_current_node = new_Const_long(_firm_types.mode_int(), value);
				}

				void visit(const ast::variable_access &node) override {
					auto var_decl = _sem_info.vardecl_annotations().at(node);
					auto field = _firm_types.get_field_entity(*var_decl);
					if (field) {
						visit_field(node, field);
					} else {
						auto type = _sem_info.type_annotations().at(node);
						auto ir_type = _firm_types.get_var_type(type);
						_var_pos =_var_ids.at(var_decl);
						_current_node = get_value(_var_pos, get_type_mode(ir_type));
					}
				}

				void visit_field(const ast::variable_access& node, ir_entity* field)
				{

					auto field_type = get_entity_type(field);


					//auto addr_mode = get_reference_offset_mode(mode_P);
					//auto field_offset = new_Const_long(addr_mode, get_entity_offset(field));
					auto ref_pointer = node.target()
					               ? get_expression_node(*node.target())
					               : get_value(0, mode_P);
					auto member = new_Member(ref_pointer, field);
					//auto addr = new_Conv(new_Add(new_Conv(ref_pointer, addr_mode), field_offset), addr_mode);

					if (_do_store) {
						//_current_node = addr;
						//new_Store(get_store(), member, )
						_current_node = member;
					} else {
						auto mem = get_store();
						auto field_mode = get_type_mode(field_type);
						auto load = new_Load(mem, member, field_mode, field_type, cons_none);
						set_store(new_Proj(load, mode_M, pn_Load_M));
						_current_node = new_Proj(load, field_mode, pn_Load_res);
					}
					//_current_node = new_Const_long(_firm_types.mode_int(), 0);
				}

				void visit(const ast::binary_expression &node) override
				{
					auto old_pos = _var_pos;

					if (is_boolean_expression(node)) {
						visit_boolean_expression(node);
					} else if (is_arithmetic_expression(node)) {
						visit_arithmetic_expression(node);
					} else if (node.type() == ast::binary_operation_type::assign) {
						visit_assignment(node);
					}

					_var_pos = old_pos;
				}

				void visit_assignment(const ast::binary_expression& expression)
				{
					assert(expression.type() == ast::binary_operation_type::assign);
					auto rhs = get_expression_node(expression.rhs());
					auto _old_do_store = _do_store;
					_do_store = true;
					auto lhs = get_expression_node(expression.lhs());
					if (is_Member(lhs)) {
						auto rhs_mode = get_irn_mode(rhs);
						auto value_type = get_type_for_mode(rhs_mode);
						auto store = new_Store(get_store(), lhs, rhs, value_type, cons_none);
						set_store(new_Proj(store, get_modeM(), pn_Store_M));
						_current_node = store;

					} else {
						// todo..
					}
					_do_store = _old_do_store;

					(void)lhs;
					(void)rhs;
				}

				void visit_boolean_expression(const ast::binary_expression& expression)
				{
					assert(is_boolean_expression(expression));
					auto lhs = get_expression_node(expression.lhs());
					auto rhs = get_expression_node(expression.rhs());
					_current_node = new_Cmp(lhs, rhs, relation_from_binary_operation_type(expression.type()));
				}

				void visit_arithmetic_expression(const ast::binary_expression& expression)
				{
					assert(is_arithmetic_expression(expression));
					auto lhs = get_expression_node(expression.lhs());
					auto rhs = get_expression_node(expression.rhs());
					auto memory = get_store();

					switch (expression.type()) {
					case ast::binary_operation_type::divide: {
						memory = get_store();
						auto div_result = new_DivRL(memory, lhs, rhs, op_pin_state_pinned);
						_current_node = new_Proj(div_result, _firm_types.mode_int(), pn_Div_res);
						set_store(new_Proj(div_result, mode_M, pn_Div_M));
						break;
					}
					case ast::binary_operation_type::modulo: {
						memory = get_store();
						auto mod_result = new_Mod(memory, lhs, rhs, op_pin_state_pinned);
						set_store(new_Proj(mod_result, mode_M, pn_Mod_M));
						_current_node = new_Proj(mod_result, _firm_types.mode_int(), pn_Mod_res);
						break;
					}
					case ast::binary_operation_type::multiply:
						_current_node = new_Mul(lhs, rhs);
						break;
					case ast::binary_operation_type::plus:
						_current_node = new_Add(lhs, rhs);
						break;
					case ast::binary_operation_type::minus:
						_current_node = new_Sub(lhs, rhs);
						break;
					default:
						MINIJAVA_NOT_REACHED();
						break;
					}
				}

				void visit(const ast::this_ref &node) override
				{
					(void) node; // node not used
					_current_node = get_value(0, mode_P);
				}

				void visit(const ast::null_constant &node) override
				{
					(void) node; // node not used
					_current_node = new_Const_long(mode_P, 0);
				}

				ir_node* current_node() const noexcept {
					return _current_node;
				}

			private:

				bool is_boolean_expression(const ast::binary_expression& expression)
				{
					auto type = expression.type();
					return type == ast::binary_operation_type::greater_equal ||
					       type == ast::binary_operation_type::greater_than ||
					       type == ast::binary_operation_type::less_equal ||
					       type == ast::binary_operation_type::less_than ||
					       type == ast::binary_operation_type::equal;
				}

				bool is_arithmetic_expression(const ast::binary_expression& expression)
				{
					auto type = expression.type();
					return type == ast::binary_operation_type::plus ||
					       type == ast::binary_operation_type::minus ||
					       type == ast::binary_operation_type::divide ||
					       type == ast::binary_operation_type::multiply ||
					       type == ast::binary_operation_type::modulo;
				}

				ir_relation relation_from_binary_operation_type(ast::binary_operation_type type)
				{
					switch (type) {
					case ast::binary_operation_type::greater_than:
						return ir_relation_greater;
					case ast::binary_operation_type::greater_equal:
						return ir_relation_greater_equal;
					case ast::binary_operation_type::less_than:
						return ir_relation_less;
					case ast::binary_operation_type::less_equal:
						return ir_relation_less_equal;
					case ast::binary_operation_type::equal:
						return ir_relation_equal;
					default:
						// should not be reached
						assert(false);
						return ir_relation_false;
					}
				}

				ir_node* get_expression_node(const ast::expression& node)
				{
					node.accept(*this);
					return this->_current_node;
				}

				bool _in_instance_method()
				{
					return _class_type != nullptr;
				}

				int _var_pos{-1};
				bool _do_store{false};

				const semantic_info& _sem_info;
				const var_id_map_type& _var_ids;
				ir_types& _firm_types;
				const ir_type* _class_type;

				ir_node* _current_node;

			};

			class method_generator final : public ast::visitor
			{

			public:

				method_generator(const semantic_info& sem_info,
				                 ir_types& firm_types, const ir_type& class_type)
						: _sem_info{sem_info}, _firm_types{firm_types},
						  _class_type{class_type}
				{}

				using ast::visitor::visit;

				void visit(const ast::instance_method& node) override
				{
					auto irg = get_current_ir_graph();
					auto method_entity = _firm_types.get_method_entity(&node);
					//auto cur_block = get_r_cur_block(irg);
					//set_r_cur_block(irg, get_irg_start_block(irg));
					auto locals = _sem_info.locals_annotations().at(node);
					auto args = get_irg_args(irg);
					auto num_params = static_cast<int>(node.parameters().size());
					auto current_id = int{1};
					set_value(0, new_Proj(args, mode_P, 0));
					for (const auto& local : locals) {
						if (current_id <= num_params) {
							set_value(current_id, new_Proj(
									args,
									get_type_mode(get_entity_type(method_entity)),
									static_cast<unsigned int>(current_id - 1)
							));
						}
						_var_ids.insert(std::make_pair(local, current_id++));
					}
					//set_r_cur_block(irg, cur_block);
					node.body().accept(*this);
				}

				void visit(const ast::local_variable_statement& node) override
				{
					assert(_var_ids.find(&node.declaration()) != _var_ids.end());
					auto node_decl = &node.declaration();
					auto pos =_var_ids.at(node_decl);

					if (node.initial_value()) {
						auto expr_node = get_expression_node(*node.initial_value());
						set_value(pos, expr_node);
					} else {
						// initialize with default zero value
						auto type = _sem_info.type_annotations().at(*node_decl);
						auto ir_type = _firm_types.get_var_type(type);
						auto null_value = new_Const_long(get_type_mode(ir_type), 0);
						set_value(pos, null_value);
					}
			}

				void visit(const ast::expression_statement& node) override
				{
					_current_node = get_expression_node(node.inner_expression());
//                  expression_generator generator{_sem_info, _var_ids, _firm_types, &_class_type};
//                  node.inner_expression().accept(generator);
					// FIXME: do something with whatever expression_generator produces
				}

				void visit(const ast::block& node) override
				{
					for (const auto& stmt : node.body()) {
						stmt->accept(*this);
						// reach unreachable code after return..
						if (!get_cur_block()) {
							break;
						}
					}
				}

				void visit(const ast::if_statement& node) override
				{
					//auto then_node = new_immBlock();
					//auto exit_node = new_immBlock();
					//auto else_node = node.else_statement() ? new_immBlock() : exit_node;
					//
					//auto cond_node = get_expression_node(node.condition());
					//add_immBlock_pred(then_node, new_Proj(cond_node, get_modeX(), pn_Cond_true));
					//add_immBlock_pred(else_node, new_Proj(cond_node, get_modeX(), pn_Cond_false));
					//
					//mature_immBlock(then_node);
					//set_cur_block(then_node);
					//visit_statement(node.then_statement());
					//if (get_cur_block()) {
					//    add_immBlock_pred(exit_node, new_Jmp());
					//}
					(void) node;
				}

				void visit(const ast::while_statement& node) override
				{
					// FIXME
					(void) node;
				}

				void visit(const ast::return_statement& node) override
				{
					auto expr = node.value();
					ir_node* ret;
					if (expr) {
						auto expression_node = get_expression_node(*node.value());
						//ir_node* results[1] = {expression_node};
						ret = new_Return(get_store(), 1, &expression_node);
					} else {
						ret = new_Return(get_store(), 0, NULL);
					}

					auto irg = get_current_ir_graph();
					add_immBlock_pred(get_irg_end_block(irg), ret);
					mature_immBlock(get_r_cur_block(irg));

					// mark as unreachable
					set_cur_block(NULL);
				}

				void visit(const ast::empty_statement& node) override
				{
					// FIXME
					(void) node;
				}

				ir_node* current_node() const noexcept {
					return _current_node;
				}

				ir_node* get_expression_node(const ast::expression& node)
				{
					expression_generator generator(_sem_info, _var_ids, _firm_types, &_class_type);
					node.accept(generator);
					return generator.current_node();
				}

			private:

				const semantic_info& _sem_info;
				ir_types& _firm_types;
				const ir_type& _class_type;

				var_id_map_type _var_ids{};

				ir_node* _current_node;

			};
		}

		void create_firm_method(const semantic_info& sem_info,
		                        ir_types& firm_types,
		                        const ir_type& class_type,
		                        const ast::instance_method& method)
		{
			auto irg = get_current_ir_graph();
			method_generator generator{sem_info, firm_types, class_type};
			method.accept(generator);
			// handle return value
			// no explicit return statement found?
			if (get_cur_block()) {
				auto store = get_store();
				auto ret = new_Return(store, 0, NULL);
				add_immBlock_pred(get_irg_end_block(irg), ret);
				mature_immBlock(get_r_cur_block(irg));
			}
		}

		void create_firm_method(const semantic_info& sem_info,
		                        ir_types& firm_types,
		                        const ir_type& class_type,
		                        const ast::main_method& method)
		{
			auto irg = get_current_ir_graph();
			method_generator generator{sem_info, firm_types, class_type};
			method.accept(generator);
			// main has no return value - so we don't need method_generator.current_node()
			// no return statement at the end => implicit return
			if (get_cur_block()) {
				auto store = get_store();
				auto ret = new_Return(store, 0, NULL);
				add_immBlock_pred(get_irg_end_block(irg), ret);
				mature_immBlock(get_r_cur_block(irg));
			}
		}

	}  // namespace sem

}  // namespace minijava
