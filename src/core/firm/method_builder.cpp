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
				                     ir_types& firm_types)
						: _sem_info{sem_info}, _var_ids{var_ids},
						  _firm_types{firm_types}
				{}

				using ast::visitor::visit;

				// FIXME

			private:

//              bool _in_instance_method()
//              {
//                  return _class_type != nullptr;
//              }

				const semantic_info& _sem_info;
				const var_id_map_type& _var_ids;
				ir_types& _firm_types;

			};

			class method_generator final// : public ast::visitor
			{

			public:

				method_generator(const semantic_info& sem_info,
				                 ir_types& firm_types)
						: _sem_info{sem_info}, _firm_types{firm_types}
				{}

				ir_node* visit(const ast::boolean_constant &node)
				{
					return new_Const_long(_firm_types.mode_boolean(), node.value());
				}

				ir_node* visit(const ast::integer_constant &node)
				{
					auto value = _sem_info.const_annotations().at(node);
					return new_Const_long(_firm_types.mode_int(), value);
				}

				ir_node* visit(const ast::binary_expression &node)
				{
					ir_node *memory;
					auto lhs = visit(node.lhs());
					auto rhs = visit(node.rhs());

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

					return (ir_node*) nullptr;
				}

				ir_node* visit(const ast::expression &node)
				{
					(void)node; // TODO
					return (ir_node*)nullptr;
				}

				ir_node* visit(const ast::this_ref &node)
				{
					(void) node; // node not used
					return get_value(0, mode_P);
				}

				ir_node* visit(const ast::null_constant &node)
				{
					(void) node; // node not used
					return new_Const_long(mode_P, 0);
				}

				void visit(const ast::local_variable_statement& node)
				{
					assert(_var_ids.find(&node.declaration()) != _var_ids.end());
					// FIXME
				}

				void visit(const ast::expression_statement& node)
				{
					expression_generator generator{_sem_info, _var_ids, _firm_types};
					node.inner_expression().accept(generator);
					// FIXME: do something with whatever expression_generator produces
				}

				void visit(const ast::block& node)
				{
					// FIXME
					(void) node;
				}

				void visit(const ast::if_statement& node)
				{
					// FIXME
					(void) node;
				}

				void visit(const ast::while_statement& node)
				{
					// FIXME
					(void) node;
				}

				void visit(const ast::return_statement& node)
				{
					// FIXME
					(void) node;
				}

				void visit(const ast::empty_statement& node)
				{
					// FIXME
					(void) node;
				}

				void visit(const ast::instance_method& node)
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
					auto graph = new_ir_graph(
							//_firm_types.get_method_entity(&node),
							nullptr, // FIXME
							num_locals
					);
					set_current_ir_graph(graph);
//                  ir_node* cur_block = get_r_cur_block(graph);
//                  set_r_cur_block(graph, get_irg_start_block(graph));
//                  ir_node* args = get_irg_args(graph);
//                  auto num_params = static_cast<int>(node.parameters().size());
//                  auto current_id = int{1};
//                  for (const auto& local : locals) {
//                      if (current_id <= num_params) {
//                          set_value(current_id, new_Proj(
//                                  args,
//                                  get_type_mode(_firm_types.get_method_type(&node)),
//                                  static_cast<unsigned int>(current_id - 1)
//                          ));
//                      }
//                      _var_ids.insert(std::make_pair(local, current_id));
//                      ++current_id;
//                  }
//                  set_r_cur_block(graph, cur_block);
					visit(node.body());
				}

			private:

				const semantic_info& _sem_info;
				ir_types& _firm_types;

				var_id_map_type _var_ids{};

			};
		}

		void create_firm_method(const semantic_info& sem_info,
		                        ir_types& firm_types,
		                        const ast::method& method)
		{
			method_generator generator{sem_info, firm_types};
//          method.accept(generator);
			(void)method;
		}

	}  // namespace sem

}  // namespace minijava
