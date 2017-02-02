#include "irg/method_builder.hpp"

#include <cassert>
#include <climits>
#include <set>
#include <type_traits>
#include <unordered_map>

#include "firm.hpp"

#include "exceptions.hpp"
#include "irg/mangle.hpp"
#include "irg/type_builder.hpp"

namespace minijava
{

	namespace irg
	{

		namespace /* anonymous */
		{

			// Converts a data node into a data node that is not of mode
			// 'b'.  In effect, this converts logical values to integers
			// and leaves other nodes alone.
			firm::ir_node* materialize(firm::ir_node*const node)
			{
				const auto mode = firm::get_irn_mode(node);
				assert(firm::mode_is_data(mode));
				if (mode != firm::mode_b) {
					return node;
				}
				const auto boolean_mode = primitive_types::get_instance().boolean_mode;
				const auto bot = firm::new_Const_long(boolean_mode, 0);
				const auto top = firm::new_Const_long(boolean_mode, 1);
				return firm::new_Mux(node, bot, top);
			}

			firm::ir_node* unmaterialize(firm::ir_node* value)
			{
				auto mode_B = primitive_types::get_instance().boolean_mode;
				if (firm::get_irn_mode(value) == mode_B) {
					return firm::new_Cmp(
							value,
							firm::new_Const_long(mode_B, 1),
							firm::ir_relation_equal
					);
				}
				return value;
			}

			using var_id_map_type = std::unordered_map<const ast::var_decl*, int>;

			class expression_generator final : public ast::visitor
			{

			public:

				expression_generator(const semantic_info& sem_info,
				                     const var_id_map_type& var_ids,
				                     const ir_types& firm_types)
						: _sem_info{sem_info}, _var_ids{var_ids},
						  _firm_types{firm_types},
				          _primitives{primitive_types::get_instance()}
				{}

				using ast::visitor::visit;

				void visit(const ast::binary_expression& node) override
				{
					if (_maybe_use_constant(node)) {
						// Use the constant provided by the front-end
					} else if (is_boolean_expression(node)) {
						visit_boolean_expression(node);
					} else if (is_arithmetic_expression(node)) {
						visit_arithmetic_expression(node);
					} else if (is_logical_expression(node)) {
						visit_logical_expression(node);
					} else if (node.type() == ast::binary_operation_type::assign) {
						visit_assignment(node);
					} else {
						MINIJAVA_NOT_REACHED();
					}
				}

				void visit(const ast::unary_expression &node) override {
					if (_maybe_use_constant(node)) {
						return;
					}
					auto rhs = get_expression_node(node.target());
					switch (node.type()) {
					case ast::unary_operation_type::minus:
						_current_node = firm::new_Minus(rhs);
						return;
					case ast::unary_operation_type::logical_not:
						_current_node = materialize(firm::new_Mux(
								unmaterialize(_current_node),
						        firm::new_Const(firm::get_tarval_b_true()),
						        firm::new_Const(firm::get_tarval_b_false())
						));
						return;
					}
					MINIJAVA_NOT_REACHED();
				}

				void visit(const ast::object_instantiation& node) override
				{
					auto type = _sem_info.type_annotations().at(node);
					auto ir_type = _firm_types.classmap.at(
							*type.info.declaration()
					);
					auto type_size = firm::get_type_size(ir_type);
					if (type_size > INT_MAX) {
						throw internal_compiler_error{
								"Cannot handle types with sizes greater than MAX_INT"
						};
					}
					firm::ir_node* arguments[2] = {
							firm::new_Const_long(firm::get_modeIs(), 1),
							firm::new_Const_long(
									firm::get_modeIs(), static_cast<long>(type_size)
							)
					};
					const auto builtin_new = _firm_types.builtins.at("new");
					auto call_node = firm::new_Call(
							firm::get_store(),
					        firm::new_Address(builtin_new),
							2,
					        arguments,
					        firm::get_entity_type(builtin_new)
					);
					firm::set_store(firm::new_Proj(call_node, firm::get_modeM(), firm::pn_Call_M));
					auto tuple = firm::new_Proj(call_node, firm::get_modeT(), firm::pn_Call_T_result);
					_current_node = firm::new_Proj(tuple, firm::get_modeP(), 0);
				}

				void visit(const ast::array_instantiation& node) override
				{
					auto type = _sem_info.type_annotations().at(node);
					auto inner_type = sem::type{type.info, type.rank - 1};
					auto inner_ir_type =  _firm_types.typemap.at(inner_type);
					auto inner_type_size = firm::get_type_size(inner_ir_type);
					if (inner_type_size > INT_MAX) {
						throw internal_compiler_error{
								"Cannot handle types with sizes greater than MAX_INT"
						};
					}
					auto extent = get_expression_node(node.extent());
					firm::ir_node* arguments[2] = {
							extent,
							firm::new_Const_long(
									firm::get_modeIs(),
									static_cast<long>(inner_type_size)
							)
					};
					const auto builtin_new = _firm_types.builtins.at("new");
					auto call_node = firm::new_Call(
							firm::get_store(),
							firm::new_Address(builtin_new),
							2,
							arguments,
							firm::get_entity_type(builtin_new)
					);
					firm::set_store(firm::new_Proj(call_node, firm::get_modeM(), firm::pn_Call_M));
					auto tuple = firm::new_Proj(call_node, firm::get_modeT(), firm::pn_Call_T_result);
					_current_node = firm::new_Proj(tuple, firm::get_modeP(), 0);
				}


				void visit(const ast::array_access& node) override
				{
					auto store = _do_store;
					_do_store = false;
					auto target = get_expression_node(node.target());
					auto index = get_expression_node(node.index());
					_do_store = store;

					auto array_ref_type = _sem_info.type_annotations().at(node.target());
					auto array_ref_ir_type =  _firm_types.typemap.at(array_ref_type);
					auto array_ir_type = firm::get_pointer_points_to_type(array_ref_ir_type);

					auto sel = firm::new_Sel(target, index, array_ir_type);

					_var_id = -1;
					auto inner_ir_type = firm::get_array_element_type(array_ir_type);
					auto mode = firm::get_type_mode(inner_ir_type);
					if (store) {
						_current_node  = sel;
					} else {
						auto mem = firm::get_store();
						auto load = firm::new_Load(mem, sel, mode, inner_ir_type, firm::cons_none);
						auto new_mem = firm::new_Proj(load, firm::mode_M, firm::pn_Load_M);
						auto value = firm::new_Proj(load, mode, firm::pn_Load_res);
						firm::set_store(new_mem);
						_current_node = value;
					}
					/*auto new_mem   = new_Proj(load, mode_M, pn_Load_M);
					auto value     = new_Proj(load, mode, pn_Load_res);
					set_store(new_mem);

					value = get_arith_value(value);
					symbolic_push(value);*/
				}

				void visit(const ast::variable_access& node) override
				{
					auto var_decl = _sem_info.vardecl_annotations().at(node);
					auto field = _firm_types.fieldmap.find(var_decl);
					if (field != _firm_types.fieldmap.end()) {
						visit_field(node, *var_decl, field->second);
					} else {
						auto type = _sem_info.type_annotations().at(node);
						auto ir_type = _firm_types.typemap.at(type);
						auto id =_var_ids.at(var_decl);
						if (_do_store) {
							// set_value(_var_id, ...) in calling method
							_var_id = id;
							_current_node = nullptr;
						} else {
							_current_node = firm::get_value(id, firm::get_type_mode(ir_type));
						}
					}
				}

				void visit(const ast::method_invocation& node) override
				{
					auto method = _sem_info.method_annotations().at(node);
					auto method_entity = _firm_types.methodmap.at(*method);
					auto method_type = firm::get_entity_type(method_entity);
					auto argc = firm::get_method_n_params(method_type);
					auto arguments = std::make_unique<firm::ir_node*[]>(argc);
					if (auto target = node.target()) {
						target->accept(*this);
						arguments[0] = _current_node;
					} else {
						// use this argument of current method
						arguments[0] = firm::get_value(0, _primitives.pointer_mode);
					}
					std::size_t i = 1;
					for (const auto& arg : node.arguments()) {
						arg->accept(*this);
						arguments[i++] = materialize(_current_node);
					}
					auto call_node = firm::new_Call(
							firm::get_store(),
							firm::new_Address(method_entity),
							static_cast<int>(argc),
							arguments.get(),
							method_type
					);
					firm::set_store(firm::new_Proj(call_node, firm::get_modeM(), firm::pn_Call_M));
					if (firm::get_method_n_ress(method_type) > 0) {
						auto tuple = firm::new_Proj(call_node, firm::get_modeT(), firm::pn_Call_T_result);
						auto res_type = firm::get_method_res_type(method_type, 0);
						_current_node = firm::new_Proj(tuple, firm::get_type_mode(res_type), 0);
					} else {
						// if the method does not return a value, _current_node
						// should never be used anyway
						_current_node = nullptr;
					}
				}

				void visit(const ast::this_ref&/* node */) override
				{
					_current_node = firm::get_value(0, _primitives.pointer_mode);
				}

				void visit(const ast::boolean_constant& node) override
				{
					const auto ok = _maybe_use_constant(node);
					assert(ok); (void) ok;
				}

				void visit(const ast::integer_constant& node) override
				{
					const auto ok = _maybe_use_constant(node);
					assert(ok); (void) ok;
				}

				void visit(const ast::null_constant&/* node */) override
				{
					_current_node = firm::new_Const_long(_primitives.pointer_mode, 0);
				}

				firm::ir_node* current_node() const noexcept
				{
					return _current_node;
				}

			private:

				void visit_field(const ast::variable_access& node,
								 const ast::var_decl& declaration,
								 firm::ir_entity* field)
				{
					firm::ir_node* member;
					if (_sem_info.is_global(&declaration)) {
						const auto ident = mangle(declaration);
						member = firm::new_Address(firm::ir_get_global(ident));
					} else {
						firm::ir_node* ref_pointer;
						if (auto target = node.target()) {
							auto old_do_store = _do_store;
							_do_store = false;
							ref_pointer = get_expression_node(*target);
							_do_store = old_do_store;
						} else {
							ref_pointer = firm::get_value(0, _primitives.pointer_mode);
						}
						member = firm::new_Member(ref_pointer, field);
					}

					auto field_type = firm::get_entity_type(field);
					auto field_mode = firm::get_type_mode(field_type);
					if (_do_store) {
						_current_node = member;
					} else {
						auto mem = firm::get_store();
						auto load = firm::new_Load(
							mem, member, field_mode, field_type, firm::cons_none
						);
						set_store(firm::new_Proj(load, firm::mode_M, firm::pn_Load_M));
						_current_node = firm::new_Proj(load, field_mode, firm::pn_Load_res);
					}
				}

				// If the front-end already extracted a constant for `node`,
				// assigns it to `_current_node` (with the correct mode) and
				// `return`s `true`.  Otherwise, `return`s `false` and leaves
				// `_current_node` unchanged.
				bool _maybe_use_constant(const ast::expression& node)
				{
					const auto pos = _sem_info.const_annotations().find(&node);
					if (pos == _sem_info.const_annotations().end()) {
						return false;
					}
					const auto type = _sem_info.type_annotations().at(node);
					if (type.rank != 0) {
						MINIJAVA_NOT_REACHED();
					} else if (type.info.is_boolean()) {
						_current_node = firm::new_Const_long(_primitives.boolean_mode, pos->second);
					} else if (type.info.is_int()) {
						_current_node = firm::new_Const_long(_primitives.int_mode, pos->second);
					} else {
						MINIJAVA_NOT_REACHED();
					}
					return true;
				}

				void visit_assignment(const ast::binary_expression& expression)
				{
					assert(expression.type() == ast::binary_operation_type::assign);
					auto old_do_store = _do_store;
					_do_store = true;
					auto lhs = get_expression_node(expression.lhs());
					auto id = _var_id;
					// reset visitor state
					_do_store = old_do_store;
					_var_id = -1;
					auto rhs = get_expression_node(expression.rhs());

					if (id != -1) {
						// local
						assert(!lhs);
						set_value(id, rhs);
						_current_node = rhs;
					} else {
						// member or array access
						assert(lhs);
						rhs = materialize(rhs);
						auto store = firm::new_Store(
							firm::get_store(),
							lhs,
							rhs,
							firm::get_type_for_mode(firm::get_irn_mode(rhs)),
							firm::cons_none
						);
						set_store(firm::new_Proj(store, firm::get_modeM(), firm::pn_Store_M));
						_current_node = rhs;
					}
				}

				void visit_boolean_expression(const ast::binary_expression& expression)
				{
					assert(is_boolean_expression(expression));
					const auto lhs = materialize(get_expression_node(expression.lhs()));
					const auto rhs = materialize(get_expression_node(expression.rhs()));
					const auto node = firm::new_Cmp(
						lhs,
						rhs,
						relation_from_binary_operation_type(expression.type())
					);
					_current_node = materialize(node);
				}

				void visit_arithmetic_expression(const ast::binary_expression& expression)
				{
					assert(is_arithmetic_expression(expression));
					auto lhs = get_expression_node(expression.lhs());
					auto rhs = get_expression_node(expression.rhs());
					auto memory = firm::get_store();
					switch (expression.type()) {
					case ast::binary_operation_type::divide: {
						auto div_result = firm::new_DivRL(
							memory,
							firm::new_Conv(lhs, firm::mode_Ls),
							firm::new_Conv(rhs, firm::mode_Ls),
							firm::op_pin_state_pinned
						);
						_current_node = firm::new_Conv(firm::new_Proj(
							div_result, firm::mode_Ls, firm::pn_Div_res
						), firm::mode_Is);
						firm::set_store(firm::new_Proj(div_result, firm::mode_M, firm::pn_Div_M));
						return;
					}
					case ast::binary_operation_type::modulo: {
						auto mod_result = firm::new_Mod(
							memory,
							firm::new_Conv(lhs, firm::mode_Ls),
							firm::new_Conv(rhs, firm::mode_Ls),
							firm::op_pin_state_pinned
						);
						firm::set_store(firm::new_Proj(mod_result, firm::mode_M, firm::pn_Mod_M));
						_current_node = firm::new_Conv(firm::new_Proj(
							mod_result, firm::mode_Ls, firm::pn_Mod_res
						), firm::mode_Is);
						return;
					}
					case ast::binary_operation_type::multiply:
						_current_node = firm::new_Mul(lhs, rhs);
						return;
					case ast::binary_operation_type::plus:
						_current_node = firm::new_Add(lhs, rhs);
						return;
					case ast::binary_operation_type::minus:
						_current_node = firm::new_Sub(lhs, rhs);
						return;
					default:
						MINIJAVA_NOT_REACHED();
					}
				}

				void visit_logical_expression(const ast::binary_expression& expression)
				{
					auto right_block = firm::new_immBlock();
					auto exit_block = firm::new_immBlock();

					auto lhs = unmaterialize(get_expression_node(expression.lhs()));
					auto cond_node = firm::new_Cond(lhs);

					if (expression.type() == ast::binary_operation_type::logical_and) {
						// goto exit_block, when lhs is false
						firm::add_immBlock_pred(right_block, firm::new_Proj(cond_node, firm::get_modeX(), firm::pn_Cond_true));
						firm::add_immBlock_pred(exit_block, firm::new_Proj(cond_node, firm::get_modeX(), firm::pn_Cond_false));
					} else if (expression.type() == ast::binary_operation_type::logical_or) {
						// goto _block, when lhs is false
						firm::add_immBlock_pred(right_block, firm::new_Proj(cond_node, firm::get_modeX(), firm::pn_Cond_false));
						firm::add_immBlock_pred(exit_block, firm::new_Proj(cond_node, firm::get_modeX(), firm::pn_Cond_true));
					} else {
						MINIJAVA_NOT_REACHED();
					}

					firm::mature_immBlock(right_block);
					firm::set_cur_block(right_block);
					auto left_mem = firm::get_store();
					auto rhs = unmaterialize(get_expression_node(expression.rhs()));
					auto right_mem = firm::get_store();

					firm::add_immBlock_pred(exit_block, firm::new_Jmp());
					firm::mature_immBlock(exit_block);
					firm::set_cur_block(exit_block);

					firm::ir_node* phi_mem_nodes[2] = {left_mem, right_mem};
					firm::ir_node* phi_nodes[2] = {lhs, rhs};
					firm::set_store(firm::new_Phi(2, phi_mem_nodes, firm::mode_M));
					_current_node = materialize(firm::new_Phi(2, phi_nodes,  firm::mode_b));
				}

				bool is_logical_expression(const ast::binary_expression& expression)
				{
					auto type = expression.type();
					return type == ast::binary_operation_type::logical_or ||
							type == ast::binary_operation_type::logical_and;
				}

				bool is_boolean_expression(const ast::binary_expression& expression)
				{
					auto type = expression.type();
					return type == ast::binary_operation_type::greater_equal ||
					       type == ast::binary_operation_type::greater_than ||
					       type == ast::binary_operation_type::less_equal ||
					       type == ast::binary_operation_type::less_than ||
					       type == ast::binary_operation_type::equal ||
					       type == ast::binary_operation_type::not_equal;
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

				firm::ir_relation relation_from_binary_operation_type(ast::binary_operation_type type)
				{
					switch (type) {
					case ast::binary_operation_type::greater_than:
						return firm::ir_relation_greater;
					case ast::binary_operation_type::greater_equal:
						return firm::ir_relation_greater_equal;
					case ast::binary_operation_type::less_than:
						return firm::ir_relation_less;
					case ast::binary_operation_type::less_equal:
						return firm::ir_relation_less_equal;
					case ast::binary_operation_type::equal:
						return firm::ir_relation_equal;
					case ast::binary_operation_type::not_equal:
						// doc:  less or greater ('not equal' for integer numbers)
						return firm::ir_relation_less_greater;
					default:
						MINIJAVA_NOT_REACHED();
					}
				}

				firm::ir_node* get_expression_node(const ast::expression& node)
				{
					node.accept(*this);
					return _current_node;
				}

				const semantic_info& _sem_info;
				const var_id_map_type& _var_ids;
				const ir_types& _firm_types;
				const primitive_types _primitives;

				int _var_id{-1};
				bool _do_store{false};
				firm::ir_node* _current_node{};
			};

			class method_generator final : public ast::visitor
			{

			public:

				method_generator(const semantic_info& sem_info,
				                 const ir_types& firm_types)
						: _sem_info{sem_info}, _firm_types{firm_types},
						  _primitives{primitive_types::get_instance()}
				{}

				using ast::visitor::visit;

				void visit(const ast::local_variable_statement& node) override
				{
					assert(_var_ids.find(&node.declaration()) != _var_ids.end());
					auto node_decl = &node.declaration();
					auto pos =_var_ids.at(node_decl);

					if (node.initial_value()) {
						auto expr_node = get_expression_node(*node.initial_value());
						firm::set_value(pos, expr_node);
					} else {
						// initialize with default zero value
						auto type = _sem_info.type_annotations().at(*node_decl);
						auto ir_type = _firm_types.typemap.at(type);
						auto null_value = firm::new_Const_long(firm::get_type_mode(ir_type), 0);
						firm::set_value(pos, null_value);
					}
				}

				void visit(const ast::expression_statement& node) override
				{
					visit_expression_node(node.inner_expression());
				}

				void visit(const ast::block& node) override
				{
					for (const auto& stmt : node.body()) {
						stmt->accept(*this);
						// reach unreachable code after return..
						if (!firm::get_cur_block()) {
							break;
						}
					}
				}

				firm::ir_node* get_compare_node(const ast::expression& condition)
				{
					auto cmp_node = get_expression_node(condition);
					if (firm::get_irn_mode(cmp_node) == _primitives.boolean_mode) {
						cmp_node = firm::new_Cmp(
								cmp_node,
								firm::new_Const_long(_primitives.boolean_mode, 1),
								firm::ir_relation_equal
						);
					}
					return firm::new_Cond(cmp_node);
				}

				void visit(const ast::if_statement& node) override
				{
					auto then_block = firm::new_immBlock();
					auto else_block = firm::new_immBlock();
					auto exit_node = firm::new_immBlock();
					size_t returns = 0;

					auto cond_node = get_compare_node(node.condition());
					add_immBlock_pred(then_block, firm::new_Proj(cond_node, firm::get_modeX(), firm::pn_Cond_true));
					add_immBlock_pred(else_block, firm::new_Proj(cond_node, firm::get_modeX(), firm::pn_Cond_false));


					// then block
					firm::mature_immBlock(then_block);
					firm::set_cur_block(then_block);
					node.then_statement().accept(*this);
					if (firm::get_cur_block()) {
						// no return statement
					    firm::add_immBlock_pred(exit_node, firm::new_Jmp());
					} else {
						returns++;
					}

					// else block
					if (node.else_statement()) {
						firm::mature_immBlock(else_block);
						firm::set_cur_block(else_block);
						node.else_statement()->accept(*this);

						if (firm::get_cur_block()) {
							// no return statement
							firm::add_immBlock_pred(exit_node, firm::new_Jmp());
						} else {
							returns++;
						}
					} else {
						firm::mature_immBlock(else_block);
						firm::add_immBlock_pred(exit_node, firm::new_r_Jmp(else_block));
					}

					// missing return statements => mature block
					if (returns < 2) {
						firm::mature_immBlock(exit_node);
						firm::set_cur_block(exit_node);
					}
				}

				void visit(const ast::while_statement& node) override
				{
					auto condition_block = firm::new_immBlock();
					auto while_block = firm::new_immBlock();
					auto exit_block = firm::new_immBlock();
					// infinite loops need this..
					firm::keep_alive(condition_block);

					// condition
					firm::add_immBlock_pred(condition_block, firm::new_Jmp());
					firm::set_cur_block(condition_block);

					auto cond_node = get_compare_node(node.condition());
					add_immBlock_pred(while_block, new_Proj(cond_node, firm::get_modeX(), firm::pn_Cond_true));
					add_immBlock_pred(exit_block, new_Proj(cond_node, firm::get_modeX(), firm::pn_Cond_false));

					// block
					firm::mature_immBlock(while_block);
					firm::set_cur_block(while_block);
					node.body().accept(*this);
					if (firm::get_cur_block()) {
						firm::add_immBlock_pred(condition_block, firm::new_Jmp());
					}

					firm::mature_immBlock(condition_block);
					firm::set_cur_block(exit_block);
					firm::mature_immBlock(exit_block);
				}

				void visit(const ast::return_statement& node) override
				{
					auto expr = node.value();
					firm::ir_node* ret;
					if (expr) {
						auto expression_node = materialize(get_expression_node(*node.value()));
						//firm::ir_node* results[1] = {expression_node};
						ret = firm::new_Return(firm::get_store(), 1, &expression_node);
					} else {
						ret = firm::new_Return(firm::get_store(), 0, nullptr);
					}

					auto irg = firm::get_current_ir_graph();
					firm::add_immBlock_pred(firm::get_irg_end_block(irg), ret);
					firm::mature_immBlock(firm::get_r_cur_block(irg));

					// mark as unreachable
					firm::set_cur_block(nullptr);
				}

				void visit(const ast::empty_statement&) override {}

				void visit(const ast::main_method& node) override
				{
					auto irg = firm::get_current_ir_graph();
					auto method_entity = _firm_types.methodmap.at(node);
					auto method_type = firm::get_entity_type(method_entity);
					auto locals = _sem_info.locals_annotations().at(node);
					auto args = firm::get_irg_args(irg);
					auto num_params = static_cast<int>(node.parameters().size());
					auto current_id = int{0};
					for (const auto& local : locals) {
						if (current_id < num_params) {
							auto param_type = firm::get_method_param_type(method_type, static_cast<size_t>(current_id));
							auto param_mode = firm::get_type_mode(param_type);
							firm::set_value(current_id, firm::new_Proj(
									args,
									param_mode,
									static_cast<unsigned int>(current_id - 1)
							));
						}
						_var_ids.insert(std::make_pair(local, current_id++));
					}
					node.body().accept(*this);
				}

				void visit(const ast::instance_method& node) override
				{
					auto irg = firm::get_current_ir_graph();
					auto method_entity = _firm_types.methodmap.at(node);
					auto method_type = firm::get_entity_type(method_entity);
					auto locals = _sem_info.locals_annotations().at(node);
					auto args = firm::get_irg_args(irg);
					auto num_params = static_cast<int>(node.parameters().size());
					auto current_id = int{1};
					firm::set_value(0, firm::new_Proj(args, _primitives.pointer_mode, 0));
					for (const auto& local : locals) {
						if (current_id <= num_params) {
							auto param_type = firm::get_method_param_type(method_type, static_cast<size_t>(current_id));
							auto param_mode = firm::get_type_mode(param_type);
							firm::set_value(current_id, firm::new_Proj(
									args,
									param_mode,
									static_cast<unsigned int>(current_id)
							));
						}
						_var_ids.insert(std::make_pair(local, current_id++));
					}
					node.body().accept(*this);
				}

			private:

				void visit_expression_node(const ast::expression& node)
				{
					expression_generator generator{
							_sem_info, _var_ids, _firm_types
					};
					node.accept(generator);
				}

				firm::ir_node* get_expression_node(const ast::expression& node)
				{
					expression_generator generator{
							_sem_info, _var_ids, _firm_types
					};
					node.accept(generator);
					return generator.current_node();
				}

				const semantic_info& _sem_info;
				const ir_types& _firm_types;
				const primitive_types _primitives;
				var_id_map_type _var_ids{};

			};
		}

		void create_firm_method(const semantic_info& sem_info,
		                        const ir_types& firm_types,
		                        const ast::instance_method& method)
		{
			auto irg = firm::get_current_ir_graph();
			method_generator generator{sem_info, firm_types};
			method.accept(generator);
			// handle return value
			// no explicit return statement found?
			if (firm::get_cur_block()) {
				auto store = firm::get_store();
				auto ret = firm::new_Return(store, 0, nullptr);
				firm::add_immBlock_pred(firm::get_irg_end_block(irg), ret);
				firm::mature_immBlock(firm::get_r_cur_block(irg));
			}
		}

		void create_builtin_method(const semantic_info & sem_info,
		                           const ir_types & firm_types,
		                           const ast::instance_method &method)
		{
			const auto irg = firm::get_current_ir_graph();
			// The AST nodes for all builtin methods have empty bodies.  Visit
			// them like ordinary methods to get the signature right.
			method_generator generator{sem_info, firm_types};
			method.accept(generator);
			const auto builtin_entity = firm_types.builtins.at(method.name().c_str());
			const auto builtin_type = firm::get_entity_type(builtin_entity);
			const auto argc = firm::get_method_n_params(builtin_type);
			auto arguments = std::make_unique<firm::ir_node*[]>(argc);
			assert(argc == method.parameters().size());
			for (std::size_t i = 0; i < argc; ++i) {
				const auto argtype = firm::get_method_param_type(builtin_type, i);
				const auto id = static_cast<int>(i + 1);
				arguments[i] = firm::get_value(id, firm::get_type_mode(argtype));
			}
			const auto call_node = firm::new_Call(
				firm::get_store(),
				firm::new_Address(builtin_entity),
				static_cast<int>(argc),
				arguments.get(),
				builtin_type
			);
			firm::set_store(firm::new_Proj(call_node, firm::get_modeM(), firm::pn_Call_M));
			const auto store = firm::get_store();
			firm::ir_node* ret = nullptr;
			if (firm::get_method_n_ress(builtin_type) > 0) {
				const auto tuple = firm::new_Proj(call_node, firm::get_modeT(), firm::pn_Call_T_result);
				const auto res_type = firm::get_method_res_type(builtin_type, 0);
				const auto res = firm::new_Proj(tuple, firm::get_type_mode(res_type), 0);
				ret = firm::new_Return(firm::get_store(), 1, &res);
			} else {
				ret = firm::new_Return(store, 0, nullptr);
			}
			firm::add_immBlock_pred(firm::get_irg_end_block(irg), ret);
			firm::mature_immBlock(firm::get_r_cur_block(irg));
		}

		void create_firm_method(const semantic_info& sem_info,
		                        const ir_types& firm_types,
		                        const ast::main_method& method)
		{
			auto irg = firm::get_current_ir_graph();
			method_generator generator{sem_info, firm_types};
			method.accept(generator);
			// main has no return value - so we don't need method_generator.current_node()
			// no return statement at the end => implicit return
			if (firm::get_cur_block()) {
				auto store = firm::get_store();
				auto ret = firm::new_Return(store, 0, nullptr);
				firm::add_immBlock_pred(firm::get_irg_end_block(irg), ret);
				firm::mature_immBlock(firm::get_r_cur_block(irg));
			}
		}

		template <typename MethodT>
		std::enable_if_t<std::is_base_of<ast::method, MethodT>{}, int>
		_get_local_var_count(const semantic_info& info, const MethodT& node)
		{
			const auto num_locals = info.locals_annotations().at(node).size();
			if (num_locals >= INT_MAX) {
				throw internal_compiler_error{
						"Cannot handle function with more than MAX_INT local variables"
				};
			}
			return (std::is_same<ast::main_method, MethodT>{})
			       ? static_cast<int>(num_locals)
			       : static_cast<int>(num_locals) + 1;
		}

		void _create_method_entity(
				const semantic_info& info,
				const ir_types& types,
				const ast::instance_method& method,
				const bool is_builtin)
		{
			const auto method_entity = types.methodmap.at(method);
			const auto irg = firm::new_ir_graph(method_entity, _get_local_var_count(info, method));
			set_current_ir_graph(irg);
			if (is_builtin) {
				create_builtin_method(info, types, method);
			} else {
				create_firm_method(info, types, method);
			}
			mature_immBlock(get_irg_end_block(irg));
			irg_finalize_cons(irg);
			assert(irg_verify(irg));
		}

		void _create_method_entity(
				const semantic_info& info,
				const ir_types& types,
		        const ast::main_method& method)
		{
			const auto method_entity = types.methodmap.at(method);
			const auto irg = firm::new_ir_graph(method_entity, _get_local_var_count(info, method));

			set_current_ir_graph(irg);
			create_firm_method(info, types, method);
			mature_immBlock(get_irg_end_block(irg));
			irg_finalize_cons(irg);
			assert(irg_verify(irg));
		}

		void create_methods(const semantic_info& info, const ir_types& types)
		{
			for (const auto& kv : info.classes()) {
				const auto clazz = kv.second.declaration();
				if (types.classmap.find(clazz) != types.classmap.end()) {
					// type was actually used in program
					for (const auto& method : clazz->instance_methods()) {
						_create_method_entity(info, types, *method, kv.second.is_builtin());
					}
				}
				for (const auto& method : clazz->main_methods()) {
					_create_method_entity(info, types, *method);
				}
			}

		}

	}  // namespace irg

}  // namespace minijava
