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
					auto type = _sem_info.type_annotations().at(node);
					auto ir_type = _firm_types.get_var_type(type);
					_var_pos =_var_ids.at(var_decl);
					_current_node = get_value(_var_pos, get_type_mode(ir_type));
				}

				void visit(const ast::binary_expression &node) override
				{
					auto old_pos = _var_pos;

					ir_node *memory;
					node.lhs().accept(*this);
					auto lhs = _current_node;
					auto lhs_pos = _var_pos;
					node.rhs().accept(*this);
					auto rhs = _current_node;

					ir_node* result = (ir_node*) nullptr;
					switch (node.type()) {
						case ast::binary_operation_type::assign:
							memory = get_store();
							set_value(lhs_pos, rhs);
							result = rhs;
							break;
						case ast::binary_operation_type::divide:
							memory  = get_store();
							result = new_DivRL(memory, lhs, rhs, op_pin_state_pinned);
							set_store(new_Proj(result, mode_M, pn_Div_M));
							result = new_Proj(result, _firm_types.mode_int(), pn_Div_res);
							break;
						case ast::binary_operation_type::multiply:
							result = new_Mul(lhs, rhs);
							break;
						case ast::binary_operation_type::plus:
							result = new_Add(lhs, rhs);
							break;
						case ast::binary_operation_type::minus:
							result = new_Sub(lhs, rhs);
							break;
						case ast::binary_operation_type::modulo:
							memory  = get_store();
							result = new_Mod(memory, lhs, rhs, op_pin_state_pinned);
							set_store(new_Proj(result, mode_M, pn_Mod_M));
							result = new_Proj(result, _firm_types.mode_int(), pn_Mod_res);
							break;
						default:
							// TODO
							// boolean mode
							break;
					}

					_current_node = result;
					_var_pos = old_pos;
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

				bool _in_instance_method()
				{
					return _class_type != nullptr;
				}

				int _var_pos{-1};

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
					auto locals = _sem_info.locals_annotations().at(node);
					auto args = get_irg_args(irg);
					auto num_params = static_cast<int>(node.parameters().size());
					auto current_id = int{1};
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
					}
				}

				void visit(const ast::if_statement& node) override
				{
					// FIXME
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
					auto store = get_store();
					if (expr) {
						auto expression_node = get_expression_node(*node.value());
						ir_node* results[1] = {expression_node};
						ret = new_Return(store, 1, results);
					} else {
						ret = new_Return(store, 0, NULL);
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
				auto has_return_type = sem_info.type_annotations().at(method).info.is_void() == false;
				auto store = get_store();
				ir_node *ret;
				if (has_return_type) {
					ir_node *results[1] = {generator.current_node()};
					ret = new_Return(store, 1, results);
				} else {
					ret = new_Return(store, 0, NULL);
				}
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
