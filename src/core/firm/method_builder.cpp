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

			private:

				bool _in_instance_method()
				{
					return _class_type != nullptr;
				}

				const semantic_info& _sem_info;
				const var_id_map_type& _var_ids;
				ir_types& _firm_types;
				const ir_type* _class_type;

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
					auto graph = new_ir_graph(
							//_firm_types.get_method_entity(&node),
							nullptr, // FIXME
							num_locals
					);
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
									get_type_mode(_firm_types.get_method_type(&node)),
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

			};
		}

		void create_firm_method(const semantic_info& sem_info,
		                        ir_types& firm_types,
		                        const ir_type& class_type,
		                        const ast::instance_method& method)
		{
			method_generator generator{sem_info, firm_types, class_type};
			method.accept(generator);
		}

	}  // namespace sem

}  // namespace minijava
