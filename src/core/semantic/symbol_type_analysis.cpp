#include "symbol_type_analysis.hpp"

#include "parser/ast.hpp"
#include "semantic/semantic_type.hpp"
#include "exceptions.hpp"
#include "std_definitions.hpp"

#include <unordered_map>
#include <stack>

namespace minijava
{

	semantic_error::semantic_error()
		: std::runtime_error{"invalid syntax"}
	{
	}

	semantic_error::semantic_error(const std::string msg)
		: std::runtime_error{msg}
	{
	}


	namespace
	{
		struct class_annotation
		{
			class_annotation(const ast::class_declaration* decl)
				: decl(decl)
				, name(decl->name())
			{
			}

			void add_method(const ast::method& decl)
			{
				using namespace std::string_literals;
				bool inserted;
				std::tie(std::ignore, inserted) = methods.emplace(decl.name(), &decl);

				if(!inserted)
				{
					throw semantic_error("Method '"s + decl.name().c_str() + "' has already been defined in '" + name.c_str() + "'!");
				}
			}

			void add_field(const ast::var_decl& decl)
			{
				using namespace std::string_literals;
				bool inserted;
				std::tie(std::ignore, inserted) = fields.emplace(decl.name(), &decl);

				if(!inserted)
				{
					throw semantic_error("Field '"s + decl.name().c_str() + "' has already been defined in '" + name.c_str() + "'!");
				}
			}

			const ast::class_declaration* decl;
			symbol name;
			std::unordered_map<symbol, const ast::method*> methods;
			std::unordered_map<symbol, const ast::var_decl*> fields;
		};

		using type_annotations = std::unordered_map<const ast::node*, semantic_type>;
		using name_annotations = std::unordered_map<const ast::node*, const ast::var_decl*>;

		struct program_annotation
		{
			class_annotation& add_class(const ast::class_declaration& decl)
			{
				using namespace std::string_literals;
				bool inserted;
				std::tie(std::ignore, inserted) = classes.emplace(decl.name(), &decl);

				if(!inserted)
				{
					throw semantic_error("Class '"s + decl.name().c_str() + "' already defined!");
				}

				decltype(class_info)::iterator it;
				std::tie(it, inserted) = class_info.emplace(&decl, &decl);
				assert(inserted);

				return it->second;
			}


			std::unordered_map<const ast::class_declaration*, class_annotation> class_info;
			std::unordered_map<symbol, const ast::class_declaration*> classes;
		};


		struct class_collector : public ast::visitor
		{
			class_collector(program_annotation& program_a, class_annotation& class_a)
				: program_a(program_a)
				, class_a(class_a)
			{
			}

			void visit(const ast::method& node) override
			{
				class_a.add_method(node);
			}

			void visit(const ast::var_decl& node) override
			{
				class_a.add_field(node);
			}

			void visit(const ast::class_declaration& node) override
			{
				do_visit_all(node.methods());
				do_visit_all(node.fields());
			}

			program_annotation& program_a;
			class_annotation& class_a;
		};

		struct program_collector : public ast::visitor
		{
			program_collector(program_annotation& program_a)
				: program_a(program_a)
			{
			}

			void visit(const ast::class_declaration& node) override
			{
				auto& class_a = program_a.add_class(node);

				class_collector collector(program_a, class_a);
				collector.do_visit(node);
			}

			void visit(const ast::program& node) override
			{
				do_visit_all(node.classes());
			}

			program_annotation& program_a;
			bool found_main_method = false;
		};

		class symbol_table
		{
			struct scope
			{
				scope* pred = nullptr;
				bool may_overwrite = true;
				std::unordered_map<symbol, const ast::var_decl*> symbols;
			};
		public:
			symbol_table()
			{
				scopes.emplace();
			}

			bool is_defined_in_dependend_scope(symbol name)
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

			const ast::var_decl* lookup(symbol name)
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

			void add_decl(const ast::var_decl& decl)
			{
				assert(!is_defined_in_dependend_scope(decl.name()));
				current()->symbols.emplace(decl.name(), &decl);
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

		private:
			std::stack<scope> scopes;
		};

		struct name_type_visitor : public ast::visitor
		{
			name_type_visitor(program_annotation& program_a)
				: program_a(program_a)
			{
			}

			const semantic_type& type_of(const ast::var_decl& decl)
			{
				auto it = type_a.find(&decl);
				if(it == type_a.end())
				{
					std::tie(it, std::ignore) = type_a.emplace(&decl, to_type(decl.var_type()));
				}
				return it->second;
			}

			const semantic_type& type_of(const ast::expression& expr)
			{
				assert(type_a.count(&expr));
				return type_a.at(&expr);
			}

			semantic_type to_type(const ast::type& ty)
			{
				struct type_visitor : public boost::static_visitor<semantic_type>
				{
					type_visitor(program_annotation& program_a, std::size_t rank)
						: program_a(program_a)
						, rank(rank)
					{
					}

					semantic_type operator()(const ast::primitive_type prim) const
					{
						switch(prim)
						{
						case ast::primitive_type::type_void:
							if(rank > 0)
							{
								throw semantic_error("Array of type 'void' is not allowed!");
							}
							return semantic_type(semantic_type_kind::kind_void, rank);
						case ast::primitive_type::type_boolean: return semantic_type(semantic_type_kind::kind_boolean, rank);
						case ast::primitive_type::type_int:     return semantic_type(semantic_type_kind::kind_int, rank);
						default:
							MINIJAVA_NOT_REACHED();
						};
					}

					semantic_type operator()(const symbol& sym) const
					{
						using namespace std::string_literals;

						auto it = program_a.classes.find(sym);
						if(it == program_a.classes.end())
						{
							throw semantic_error("Can not resolve typename '"s + sym.c_str() + "'");
						}

						return semantic_type(it->second, rank);
					}

					program_annotation& program_a;
					std::size_t rank;
				};

				return boost::apply_visitor(type_visitor{program_a, ty.rank()}, ty.name());
			}

			void check_not_void(const semantic_type& ty)
			{
				if(ty.kind() == semantic_type_kind::kind_void)
				{
					throw semantic_error("Type 'void' is only allowed as return type!");
				}
			}

			void check_type(const semantic_type& expected, const semantic_type& actual)
			{
				using namespace std::string_literals;
				if(!is_assginable(actual, expected))
				{
					throw semantic_error("Expected type '"s + expected.to_string() + "' but actual type is '" + actual.to_string() + "'");
				}
			}

			bool is_assginable(const semantic_type& from, const semantic_type& to)
			{
				return from == to
					|| (from.kind() == semantic_type_kind::kind_null_reference && to.is_reference());
			}

			semantic_type current_this_type()
			{
				assert(cur_class);
				return semantic_type(cur_class);
			}

			void visit(const ast::var_decl& decl) override
			{
				using namespace std::string_literals;
				if(symbols.is_defined_in_dependend_scope(decl.name()))
				{
					throw semantic_error("Variable '"s + decl.name().c_str() + "' has already been defined in the current scope!");
				}
				auto ty = type_of(decl);
				check_not_void(ty);
				symbols.add_decl(decl);
			}

			void visit(const ast::binary_expression& node) override
			{
				do_visit(node.lhs());
				do_visit(node.rhs());
				auto lhs_type = type_of(node.lhs());
				auto rhs_type = type_of(node.rhs());
				auto ret_type = resolve_binary_operator(node.type(), lhs_type, rhs_type);
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
				auto ret_type = resolve_unary_operator(node.type(), in_type);
				if(!ret_type)
				{
					throw semantic_error("Wrong type for unary operation");
				}
				type_a.emplace(&node, *ret_type);
			}

			void visit(const ast::object_instantiation& node) override
			{
				using namespace std::string_literals;

				auto class_name = node.class_name();
				auto it = program_a.classes.find(class_name);
				if(it == program_a.classes.end())
				{
					throw semantic_error("Can not resolve typename '"s + class_name.c_str() + "'");
				}

				auto type = semantic_type(it->second);
				type_a.emplace(&node, type);
			}

			void visit(const ast::array_instantiation& node) override
			{
				auto type = to_type(node.array_type());
				check_not_void(type);
				do_visit(node.extent());
				auto extent_type = type_of(node.extent());
				if(extent_type != semantic_type_kind::kind_int)
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
				check_type(semantic_type_kind::kind_int, idx_type);

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
					if(!ty.is_objref())
					{
						throw semantic_error(ty.to_string() + " has no fields!");
					}
					// find the field
					auto clazz = program_a.class_info.at(ty.decl());
					auto it = clazz.fields.find(node.name());
					if(it == clazz.fields.end())
					{
						throw semantic_error(ty.to_string() + " has no field '" + node.name().c_str() +"'");
					}
					name_a.emplace(&node, it->second);
					type_a.emplace(&node, type_of(*(it->second)));
				}else{
					const ast::var_decl* decl = symbols.lookup(node.name());

					if(!decl)
					{
						throw semantic_error("No variable '"s + node.name().c_str() + "' defined in current scope");
					}

					name_a.emplace(&node, decl);
					type_a.emplace(&node, type_of(*decl));
				}
			}

			void visit(const ast::method_invocation& node) override
			{
				visit_expression(node);
			}

			void visit(const ast::this_ref& node) override
			{
				assert(!type_a.count(&node));
				type_a.emplace(&node, current_this_type());
			}

			void visit(const ast::boolean_constant& node) override
			{
				assert(!type_a.count(&node));
				type_a.emplace(&node, semantic_type(semantic_type_kind::kind_boolean));
			}

			void visit(const ast::integer_constant& node) override
			{
				assert(!type_a.count(&node));
				type_a.emplace(&node, semantic_type(semantic_type_kind::kind_int));
			}

			void visit(const ast::null_constant& node) override
			{
				assert(!type_a.count(&node));
				type_a.emplace(&node, semantic_type(semantic_type_kind::kind_null_reference));
			}

			void visit(const ast::local_variable_statement& node) override
			{
				do_visit(node.declaration());
				if(node.initial_value())
				{
					do_visit(*node.initial_value());
					auto& ty = type_of(node.declaration());
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
				check_type(semantic_type(semantic_type_kind::kind_boolean), type_of(node.condition()));
				do_visit(node.then_statement());
				if(node.else_statement())
					do_visit(*node.else_statement());
			}

			void visit(const ast::while_statement& node) override
			{
				do_visit(node.condition());
				check_type(semantic_type(semantic_type_kind::kind_boolean), type_of(node.condition()));
				do_visit(node.body());
			}

			void visit(const ast::return_statement& node) override
			{
				//node.value()
				//do_visit()
				(void)node;
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
				cur_method = &node;
				symbols.enter_scope(true);
				to_type(node.return_type());
				do_visit_all(node.parameters());
				do_visit(node.body());
				symbols.leave_scope();
				cur_method = nullptr;
			}

			void visit(const ast::class_declaration& node) override
			{
				assert(!cur_class);
				cur_class = &node;
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
			const ast::method* cur_method = nullptr;
			const ast::class_declaration* cur_class = nullptr;
			program_annotation& program_a;
			type_annotations type_a;
			name_annotations name_a;
		};
	}


	void analyse_program(const ast::program& prog)
	{
		program_annotation program_info;

		program_collector collector(program_info);
		collector.do_visit(prog);

		name_type_visitor visitor(program_info);
		visitor.do_visit(prog);
	}
}
