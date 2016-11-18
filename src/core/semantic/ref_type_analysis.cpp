#include "ref_type_analysis.hpp"

#include "exceptions.hpp"
#include "parser/ast.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/symbol_def.hpp"
#include "semantic/buildins.hpp"

#include <stack>
#include <utility>

namespace minijava
{
	namespace semantic
	{
		namespace
		{
			class symbol_table
			{
				struct scope
				{
					scope* pred = nullptr;
					bool may_overwrite = true;
					std::unordered_map<symbol, const symbol_def*> symbols;
				};
			public:
				symbol_table()
				{
					scopes.emplace();
				}

				bool is_defined_in_dependend_scope(symbol name) const
				{
					for(auto scope = current(); scope; scope = scope->pred)
					{
						if(scope->symbols.count(name))
							return true;

						if(scope->may_overwrite)
							break;
					}
					return false;
				}

				const symbol_def* lookup(symbol name) const
				{
					for(auto scope = current(); scope; scope = scope->pred)
					{
						auto it = scope->symbols.find(name);

						if(it != scope->symbols.end())
						{
							return it->second;
						}
					}
					return nullptr;
				}

				void add_def(const symbol_def& def)
				{
					assert(!is_defined_in_dependend_scope(def.name()));
					current()->symbols.emplace(def.name(), &def);
				}

				void enter_scope(bool may_overwrite)
				{
					auto pred = current();
					scopes.emplace();
					auto cur = current();
					cur->pred = pred->symbols.empty() && pred->may_overwrite == may_overwrite
								? pred->pred
								: pred;
					cur->may_overwrite = may_overwrite;
				}

				void leave_scope()
				{
					scopes.pop();
					assert(!scopes.empty());
				}

			private:
				scope* current()
				{
					assert(!scopes.empty());
					return &scopes.top();
				}

				const scope* current() const
				{
					assert(!scopes.empty());
					return &scopes.top();
				}

			private:
				std::stack<scope> scopes;
			};

			struct name_type_visitor : public ast::visitor
			{
				name_type_visitor(const type_system& typesystem, const globals_list& globals, def_annotations& def_a)
					: _typesystem(typesystem)
					, _def_a(def_a)
				{
					for(auto&& gdef : globals)
					{
						auto def = std::make_unique<global_def>(gdef.first, gdef.second);
						symbols.add_def(*def);
						_def_a.store(std::move(def));
					}
				}

				t_type type_of(const ast::var_decl& decl)
				{
					return _typesystem.resolve(decl.var_type());
				}

				const t_type& type_of(const ast::expression& expr)
				{
					assert(type_a.count(&expr));
					return type_a.at(&expr);
				}

				void check_not_void(const t_type& type)
				{
					if(type == type_system::t_void())
					{
						throw semantic_error("Type 'void' is only allowed as return type!");
					}
				}

				void check_type(const t_type& expected, const t_type& actual)
				{
					using namespace std::string_literals;
					if(!_typesystem.is_assignable(actual, expected))
					{
						throw semantic_error("Expected type '"s + expected.to_string() + "' but actual type is '" + actual.to_string() + "'");
					}
				}

				t_type current_this_type()
				{
					assert(cur_class);
					return cur_class->type();
				}

				void visit(const ast::var_decl& node) override
				{
					using namespace std::string_literals;
					if(symbols.is_defined_in_dependend_scope(node.name()))
					{
						throw semantic_error("Variable '"s + node.name().c_str() + "' has already been defined in the current scope!");
					}
					if(cur_method) // fields and parameter already have definitions
					{
						auto ty = type_of(node);
						check_not_void(ty);
						auto vdef = std::make_unique<var_def>(node.name(), ty, *cur_method, &node);
						symbols.add_def(*vdef);
						_def_a.store(std::move(vdef));
					}else{
						symbols.add_def(_def_a[node]);
					}
				}

				void visit(const ast::binary_expression& node) override
				{
					do_visit(node.lhs());
					do_visit(node.rhs());
					auto lhs_type = type_of(node.lhs());
					auto rhs_type = type_of(node.rhs());
					auto ret_type = buildins::resolve_binary_operator(node.type(), lhs_type, rhs_type, _typesystem);
					if(!ret_type)
					{
						throw semantic_error("Wrong type for binary operation");
					}
					type_a.emplace(&node, *ret_type);
				}

				void visit(const ast::unary_expression& node) override
				{
					do_visit(node.target());
					auto in_type = type_of(node.target());
					auto ret_type = buildins::resolve_unary_operator(node.type(), in_type);
					if(!ret_type)
					{
						throw semantic_error("Wrong type for unary operation");
					}
					type_a.emplace(&node, *ret_type);
				}

				void visit(const ast::object_instantiation& node) override
				{
					auto type = _typesystem.resolve(node.class_name());
					type_a.emplace(&node, type);
				}

				void visit(const ast::array_instantiation& node) override
				{
					auto type = _typesystem.resolve(node.array_type());
					check_not_void(type);
					do_visit(node.extent());
					auto extent_type = type_of(node.extent());
					if(extent_type != type_system::t_int())
					{
						throw semantic_error("Expected int expression for array extent");
					}
					type_a.emplace(&node, type);
				}

				void visit(const ast::array_access& node) override
				{
					do_visit(node.target());
					const auto arr_type = type_of(node.target());
					if(!arr_type.is_array())
					{
						throw semantic_error("Expected an array expression but found type '" + arr_type.to_string() + "'");
					}

					do_visit(node.index());
					const auto idx_type = type_of(node.index());
					check_type(type_system::t_int(), idx_type);

					type_a.emplace(&node, arr_type.subrank());
				}

				void visit(const ast::variable_access& node) override
				{
					using namespace std::string_literals;
					if(node.target())
					{
						// access the field of an object
						do_visit(*node.target());
						auto ty = type_of(*node.target());
						if(!ty.has_member())
						{
							throw semantic_error(ty.to_string() + " has no fields!");
						}
						// find the field
						auto& clazz = ty.objref();
						auto field = clazz.field(node.name());
						if(!field)
						{
							throw semantic_error(ty.to_string() + " has no field '" + node.name().c_str() +"'");
						}
						name_a.emplace(&node, field);
						type_a.emplace(&node, field->type());
					}else{
						auto* def = symbols.lookup(node.name());

						if(!def)
						{
							throw semantic_error("No variable '"s + node.name().c_str() + "' defined in current scope");
						}

						name_a.emplace(&node, def);
						type_a.emplace(&node, def->type());
					}
				}

				void visit(const ast::method_invocation& node) override
				{
					using namespace std::string_literals;
					t_type target_type = cur_class->type();
					if(node.target())
					{
						// access the methods of an object
						do_visit(*node.target());
						target_type = type_of(*node.target());

						if(!target_type.has_member())
						{
							throw semantic_error(target_type.to_string() + " has no methods!");
						}
					}

					// find the method
					const auto& clazz = target_type.objref();
					const auto* method = clazz.method(node.name());
					if(!method)
					{
						throw semantic_error(target_type.to_string() + " has no method '" + node.name().c_str() +"'");
					}

					// check parameter
					do_visit_all(node.arguments());
					const auto& formal_params = method->parameters();
					const auto expected_size = formal_params.size();
					const auto actual_size = node.arguments().size();

					if(expected_size != actual_size)
					{
						throw semantic_error("Expected '" + std::to_string(expected_size) + "' parameters in call to '"
						                     + method->name().c_str() + "' but found " + std::to_string(actual_size));
					}

					for(std::size_t i = 0; i < actual_size; ++i)
					{
						const auto* param = method->parameters()[i];
						const auto& arg = *node.arguments()[i];
						auto expected_type = param->type();
						auto actual_type = type_of(arg);
						if(!_typesystem.is_assignable(actual_type, expected_type))
						{
							throw semantic_error("Expected type " + expected_type.to_string() + "' in parameter " + std::to_string(i + 1) + " in call to '"
							                     + method->name().c_str() + "' but found " + actual_type.to_string());
						}
					}

					name_a.emplace(&node, method);
					type_a.emplace(&node, method->type());
				}

				void visit(const ast::this_ref& node) override
				{
					assert(!type_a.count(&node));
					type_a.emplace(&node, current_this_type());
				}

				void visit(const ast::boolean_constant& node) override
				{
					assert(!type_a.count(&node));
					type_a.emplace(&node, type_system::t_boolean());
				}

				void visit(const ast::integer_constant& node) override
				{
					assert(!type_a.count(&node));
					type_a.emplace(&node, type_system::t_int());
				}

				void visit(const ast::null_constant& node) override
				{
					assert(!type_a.count(&node));
					type_a.emplace(&node, type_system::t_null());
				}

				void visit(const ast::local_variable_statement& node) override
				{
					do_visit(node.declaration());
					const auto& ty = type_of(node.declaration());
					if(node.initial_value())
					{
						do_visit(*node.initial_value());
						check_type(ty, type_of(*node.initial_value()));
					}
				}

				void visit(const ast::expression_statement& node) override
				{
					do_visit(node.inner_expression());
				}

				void visit(const ast::block& node) override
				{
					symbols.enter_scope(false);
					do_visit_all(node.body());
					symbols.leave_scope();
				}

				void visit(const ast::if_statement& node) override
				{
					do_visit(node.condition());
					check_type(type_system::t_boolean(), type_of(node.condition()));
					do_visit(node.then_statement());
					if(node.else_statement())
						do_visit(*node.else_statement());
				}

				void visit(const ast::while_statement& node) override
				{
					do_visit(node.condition());
					check_type(type_system::t_boolean(), type_of(node.condition()));
					do_visit(node.body());
				}

				void visit(const ast::return_statement& node) override
				{
					using namespace std::string_literals;
					if(cur_method->type() == type_system::t_void())
					{
						if(node.value())
						{
							throw semantic_error("Method '"s + cur_method->name().c_str() + "' has return type void and can not return a value");
						}
					}else{
						if(!node.value())
						{
							throw semantic_error("Expected return statement to return a value");
						}
						do_visit(*node.value());
						check_type(cur_method->type(), type_of(*node.value()));
					}
				}

				void visit(const ast::empty_statement&) override
				{
					/* nothing to do */
				}

				void visit(const ast::main_method& node) override
				{
					(void)node;
				}

				void visit(const ast::method& node) override
				{
					assert(!cur_method);
					symbols.enter_scope(true);

					do_visit(node.return_type());
					do_visit_all(node.parameters());

					cur_method = &_def_a[node];
					do_visit(node.body());
					cur_method = nullptr;

					symbols.leave_scope();
				}

				void visit(const ast::class_declaration& node) override
				{
					assert(!cur_class);
					cur_class = &_def_a[node];
					symbols.enter_scope(true);

					do_visit_all(node.fields());
					do_visit_all(node.methods());

					symbols.leave_scope();
					cur_class = nullptr;
				}

				void visit(const ast::program& node) override
				{
					do_visit_all(node.classes());
				}


				symbol_table symbols;
				bool cur_parameter = false;
				const method_def* cur_method = nullptr;
				const class_def* cur_class = nullptr;
				const type_system& _typesystem;
				def_annotations& _def_a;
				std::unordered_map<const ast::expression*, t_type> type_a;
				std::unordered_map<const ast::node*, const symbol_def*> name_a;
			};
		}

		type_annotation::type_annotation(std::unordered_map<const ast::expression*, t_type> annotations)
			: _types(std::move(annotations))
		{
		}

		t_type type_annotation::operator[](const ast::expression& node) const
		{
			assert(_types.count(&node));
			return _types.at(&node);
		}

		const std::unordered_map<const ast::expression*, t_type>& type_annotation::types() const
		{
			return _types;
		}


		ref_annotation::ref_annotation(std::unordered_map<const ast::node*, const symbol_def*> annotations)
			: _refs(std::move(annotations))
		{
		}

		const method_def& ref_annotation::operator[](const ast::method_invocation& node) const
		{
			assert(_refs.count(&node));
			assert(dynamic_cast<const method_def*>(_refs.at(&node)));
			return static_cast<const method_def&>(*_refs.at(&node));
		}

		const symbol_def& ref_annotation::operator[](const ast::variable_access& node) const
		{
			assert(_refs.count(&node));
			return *_refs.at(&node);
		}

		std::pair<type_annotation, ref_annotation> analyse_program(const ast::program& prog, const type_system& typesystem, def_annotations& def_a)
		{
			return analyse_program(prog, {}, typesystem, def_a);
		}

		std::pair<type_annotation, ref_annotation> analyse_program(const ast::program& prog, const globals_list& globals, const type_system& typesystem, def_annotations& def_a)
		{
			name_type_visitor visitor(typesystem, globals, def_a);
			visitor.do_visit(prog);

			return {std::move(visitor.type_a), std::move(visitor.name_a)};
		}
	}
}
