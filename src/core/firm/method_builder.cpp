#include "firm/method_builder.hpp"

#include <set>

#include "libfirm/firm.h"

#include "exceptions.hpp"
#include "firm/builder.hpp"
#include "semantic/attribute.hpp"

namespace minijava
{

	namespace firm
	{

		namespace /* anonymous */
		{

			class expression_generator final : public ast::visitor
			{

			public:

				expression_generator(const semantic_info::const_attributes& consts,
				                     ir_types& firm_types, const ir_type* class_type)
						: _consts{consts}, _firm_types{firm_types},
						  _class_type{class_type}
				{}

				using ast::visitor::visit;

				// FIXME

			private:

				bool _in_instance_method()
				{
					return _class_type != nullptr;
				}

				const sem::const_attributes& _consts;
				ir_types& _firm_types;
				const ir_type* _class_type;

			};

			class method_generator final : public ast::visitor
			{

			public:

				method_generator(const std::set<const ast::var_decl*>& locals,
				                 const semantic_info::const_attributes& consts,
				                 ir_types& firm_types, const ir_type& class_type)
						: _locals{locals}, _consts{consts},
						  _firm_types{firm_types}, _class_type{class_type}
				{}

				using ast::visitor::visit;

				void visit(const ast::local_variable_statement& node) override
				{
					assert(_var_ids.find(&node.declaration()) != _var_ids.end());
					// FIXME
				}

				void visit(const ast::expression_statement& node) override
				{
					expression_generator generator{_consts, _firm_types, &_class_type};
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
					auto num_locals_ = _locals.size();
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
					for (const auto& local : _locals) {
						if (current_id <= num_params) {
							set_value(current_id, new_Proj(
									args,
									get_type_mode(_firm_types.get_method_type(&node)),
									static_cast<unsigned int>(current_id - 1)
							));
						}
						_var_ids.put(*local, current_id);
						++current_id;
					}
					set_r_cur_block(graph, cur_block);
					visit(node.body());
				}

			private:

				const std::set<const ast::var_decl*>& _locals;
				const sem::const_attributes& _consts;
				ir_types& _firm_types;
				const ir_type& _class_type;

				ast_attributes<int, ast_node_filter<ast::var_decl>> _var_ids{};

			};
		}

		void create_firm_method(const semantic_info::locals_attributes& locals,
		                        const semantic_info::const_attributes& consts,
		                        ir_types& firm_types,
		                        const ir_type& class_type,
		                        const ast::instance_method& method)
		{
			method_generator generator{locals.at(method), consts, firm_types, class_type};
			method.accept(generator);
		}

	}  // namespace sem

}  // namespace minijava
