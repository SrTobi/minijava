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

				// FIXME

				void visit(const ast::boolean_constant &node) override
				{
					current_node = new_Const_long(_firm_types.mode_boolean(), node.value());
				}

				void visit(const ast::integer_constant &node) override
				{
					auto value = _sem_info.const_annotations().at(node);
					current_node = new_Const_long(_firm_types.mode_int(), value);
				}

				void visit(const ast::binary_expression &node) override
				{
					ir_node *memory;
					node.lhs().accept(*this);
					auto lhs = current_node;
					node.rhs().accept(*this);
					auto rhs = current_node;

					ir_node* result;
					switch (node.type()) {
						case ast::binary_operation_type::assign:
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

					current_node = (ir_node*) nullptr;
				}

				void visit_expression(const ast::expression &node) override
				{
					(void)node; // TODO
					current_node = (ir_node*)nullptr;
				}

				void visit(const ast::this_ref &node) override
				{
					(void) node; // node not used
					current_node = get_value(0, mode_P);
				}

				void visit(const ast::null_constant &node) override
				{
					(void) node; // node not used
					current_node = new_Const_long(mode_P, 0);
				}

			private:

				bool _in_instance_method()
				{
					return _class_type != nullptr;
				}

				const semantic_info& _sem_info;
				const var_id_map_type& _var_ids;
				ir_types& _firm_types;
				const ir_type* _class_type;

				ir_node* current_node;

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

				void visit(const ast::local_variable_statement& node) override
				{
					assert(_var_ids.find(&node.declaration()) != _var_ids.end());
					// FIXME
				}

				void visit(const ast::expression_statement& node) override
				{
					expression_generator generator{_sem_info, _var_ids, _firm_types, &_class_type};
					node.inner_expression().accept(generator);
					// FIXME: do something with whatever expression_generator produces
				}

				void visit(const ast::block& node) override
				{
					// FIXME
					(void) node;
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
					// FIXME
					(void) node;
				}

				void visit(const ast::empty_statement& node) override
				{
					// FIXME
					(void) node;
				}

				ir_node* current_node() const noexcept {
					return _current_node;
				void visit(const ast::instance_method& node) override
				{
					auto locals = _sem_info.locals_annotations().at(node);
					auto num_locals_ = locals.size();
					_var_ids.reserve(num_locals_);
					auto const max_locals = std::numeric_limits<int>::max();
					// add 1 for "this" parameter
					if (__builtin_add_overflow(num_locals_, 1, &num_locals_) ||
					    num_locals_ > static_cast<std::size_t>(max_locals)) {
						MINIJAVA_THROW_ICE_MSG(
								minijava::internal_compiler_error,
								"Cannot handle functions with more than MAX_INT"
										" local variables"
						);
					}
					auto num_locals = static_cast<int>(num_locals_);
					auto method_entity = _firm_types.get_method_entity(&node);
					auto graph = new_ir_graph(method_entity, num_locals);
					set_current_ir_graph(graph);
					ir_node* cur_block = get_r_cur_block(graph);
					set_r_cur_block(graph, get_irg_start_block(graph));
					ir_node* args = get_irg_args(graph);
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
						_var_ids.insert(std::make_pair(local, current_id));
						++current_id;
					}
					set_r_cur_block(graph, cur_block);
					visit(node.body());
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
			method.body().accept(generator);
			// handle return value
			auto has_return_type = sem_info.type_annotations().at(method).info.is_void() == false;
			auto store = get_store();
			ir_node* ret;
			if (has_return_type) {
				ir_node* results[1] = {generator.current_node()};
				ret = new_Return(store, 0, results);
			} else {
				ret = new_Return(store, 0, 0);
			}
			add_immBlock_pred(get_irg_end_block(irg), ret);
			mature_immBlock(get_r_cur_block(irg));
		}

		void create_firm_method(const semantic_info& sem_info,
		                        ir_types& firm_types,
		                        const ir_type& class_type,
		                        const ast::main_method& method)
		{
			auto irg = get_current_ir_graph();
			method_generator generator{sem_info, firm_types, class_type};
			method.body().accept(generator);
			// main has no return value - so we don't need method_generator.current_node()
			auto store = get_store();
			auto ret = new_Return(store, 0, 0);
			add_immBlock_pred(get_irg_end_block(irg), ret);
			mature_immBlock(get_r_cur_block(irg));
		}

	}  // namespace sem

}  // namespace minijava
