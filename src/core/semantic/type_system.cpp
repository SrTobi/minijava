#include "type_system.hpp"

#include "exceptions.hpp"
#include "symbol_def.hpp"
#include "semantic/semantic_error.hpp"

namespace minijava
{
	namespace semantic
	{
		namespace {
			struct member_collector : public ast::visitor
			{
				member_collector(type_system& typesystem, class_def& clazz_def, const t_type& string_type)
					: typesystem(typesystem)
					, clazz_def(clazz_def)
					, string_type(string_type)
				{
				}

				using ast::visitor::visit;

				void visit(const ast::method& node) override
				{
					auto ret_type = typesystem.resolve(node.return_type());
					auto* method_d = clazz_def.new_method(ret_type, node.name(), &node);

					for(auto&& param : node.parameters())
					{
						auto param_type = typesystem.resolve(param->var_type());
						method_d->add_parameter(param_type, param->name(), param.get());
					}
				}

				void visit(const ast::main_method& node) override
				{
					if(node.name().c_str() != std::string("main"))
					{
						throw semantic_error("static method must have name main");
					}

					auto* method_d = clazz_def.new_method(type_system::t_void(), node.name(), &node, true);
					method_d->add_parameter(string_type.with_rank(1), node.argname(), nullptr);
				}

				void visit(const ast::var_decl& node) override
				{
					auto field_type = typesystem.resolve(node.var_type());
					clazz_def.new_field(field_type, node.name(), &node);
				}

				virtual void visit(const ast::class_declaration& node) override
				{
					do_visit_all(node.fields());
					do_visit_all(node.methods());
					do_visit_all(node.main_methods());
				}

				type_system& typesystem;
				class_def& clazz_def;
				const t_type& string_type;
			};
		}

		namespace detail {
			struct type_base
			{
				virtual ~type_base() = default;
				virtual bool is_reference() const noexcept = 0;
				virtual bool is_objref() const noexcept = 0;
				virtual bool is_instantiable() const noexcept = 0;
				virtual const class_def& get_objref() const = 0;
				virtual const std::string& to_string() const = 0;
			};

			struct primitive_type_base : public type_base
			{
				primitive_type_base(const std::string& name)
					: _name(name)
				{}

				virtual bool is_reference() const noexcept override
				{
					return false;
				}

				virtual bool is_objref() const noexcept override
				{
					return false;
				}

				virtual bool is_instantiable() const noexcept override
				{
					return true;
				}

				virtual const class_def& get_objref() const override
				{
					MINIJAVA_THROW_ICE_MSG(::minijava::internal_compiler_error, "A non object type should not be asked for an objref");
				}

				virtual const std::string& to_string() const override
				{
					return _name;
				}

				std::string _name;
			};

			struct null_type_base : public primitive_type_base
			{
				null_type_base()
					: primitive_type_base("null")
				{
				}

				virtual bool is_reference() const noexcept override
				{
					return true;
				}

				virtual bool is_instantiable() const noexcept override
				{
					return false;
				}
			};

			struct void_type_base : public primitive_type_base
			{
				void_type_base()
					: primitive_type_base("void")
				{
				}

				virtual bool is_instantiable() const noexcept override
				{
					return false;
				}
			};

			struct class_type_base : public type_base, public class_def
			{
				class_type_base(const symbol& name, const ast::class_declaration* decl, def_annotations& def_a)
					: class_def(name, decl, def_a)
					, _name(name.c_str())
				{
				}

				virtual bool is_reference() const noexcept override
				{
					return true;
				}

				virtual bool is_objref() const noexcept override
				{
					return true;
				}

				virtual bool is_instantiable() const noexcept override
				{
					return true;
				}

				virtual const class_def& get_objref() const override
				{
					return *this;
				}

				virtual const std::string& to_string() const override
				{
					return _name;
				}

				virtual t_type type() const noexcept override
				{
					return t_type(this, 0);
				}

				std::string _name;
			};
		}


		t_type::t_type(const detail::type_base* inner, std::size_t rank)
			: _inner(inner)
			, _rank(rank)
		{
			assert(inner);
		}

		std::size_t t_type::rank() const noexcept
		{
			return _rank;
		}

		bool t_type::is_reference() const noexcept
		{
			return is_array() || _inner->is_reference();
		}

		bool t_type::is_array() const noexcept
		{
			return rank() > 0;
		}

		bool t_type::is_objref() const noexcept
		{
			return !is_array() && _inner->is_objref();
		}

		bool t_type::has_member() const noexcept
		{
			return is_objref();
		}

		bool t_type::is_instantiable() const noexcept
		{
			return _inner->is_instantiable();
		}

		t_type t_type::pure() const noexcept
		{
			return with_rank(0);
		}

		t_type t_type::subrank() const
		{
			assert(rank() > 0);
			return with_rank(rank() - 1);
		}

		t_type t_type::with_rank(std::size_t rank) const
		{
			return t_type(_inner, rank);
		}

		const class_def& t_type::objref() const
		{
			return _inner->get_objref();
		}

		std::string t_type::to_string() const
		{

			std::string result = _inner->to_string();

			for(std::size_t i = 0; i < rank(); ++i)
			{
				result += "[]";
			}
			return result;
		}


		bool operator==(const t_type& lhs, const t_type& rhs)
		{
			return lhs.rank() == rhs.rank() && lhs._inner == rhs._inner;
		}

		bool operator!=(const t_type& lhs, const t_type& rhs)
		{
			return !(lhs == rhs);
		}


		type_system::type_system(def_annotations& def_a)
			: _def_a(def_a)
		{
		}

		t_type type_system::resolve(const ast::type& type) const
		{
			struct type_visitor : public boost::static_visitor<t_type>
			{
				type_visitor(const type_system& typesystem, std::size_t rank)
					: typesystem(typesystem)
					, rank(rank)
				{
				}

				t_type operator()(const ast::primitive_type prim) const
				{
					switch(prim)
					{
					case ast::primitive_type::type_void:
						if(rank > 0)
						{
							throw semantic_error("Array of type 'void' is not allowed!");
						}
						return t_void();
					case ast::primitive_type::type_boolean: return t_boolean_array(rank);
					case ast::primitive_type::type_int:     return t_int_array(rank);
					default:
						MINIJAVA_NOT_REACHED();
					};
				}

				t_type operator()(const symbol& sym) const
				{
					return typesystem.resolve(sym).with_rank(rank);
				}

				const type_system& typesystem;
				std::size_t rank;
			};

			return boost::apply_visitor(type_visitor{*this, type.rank()}, type.name());
		}

		t_type type_system::resolve(const symbol& name) const
		{
			using namespace std::string_literals;

			auto* clazz = resolve_class(name);
			if(!clazz)
			{
				throw semantic_error("Can not resolve typename '"s + name.c_str() + "'");
			}

			return clazz->type();
		}

		const class_def* type_system::resolve_class(const symbol& name) const
		{
			auto it = _classes.find(name);
			if(it != _classes.end())
			{
				return it->second;
			}
			return nullptr;
		}

		bool type_system::is_assignable(const t_type& from, const t_type& to) const
		{
			return from == to || (from == t_null() && to.is_reference());
		}

		const std::unordered_map<symbol, const class_def*>& type_system::classes() const
		{
			return _classes;
		}

		class_def* type_system::new_class(const symbol& name, const ast::class_declaration* decl)
		{
			using namespace std::string_literals;
			auto def = std::make_unique<detail::class_type_base>(name, decl, _def_a);
			auto* ptr = def.get();
			bool inserted = false;
			std::tie(std::ignore, inserted) = _classes.emplace(name, ptr);
			if(!inserted)
			{
				throw semantic_error("Class '"s + name.c_str() + "' already defined!");
			}
			_def_a.store(std::move(def));
			return ptr;
		}

		t_type type_system::t_null()
		{
			static detail::null_type_base null_inst;
			return t_type(&null_inst, 0);
		}

		t_type type_system::t_void()
		{
			static detail::void_type_base void_inst;
			return t_type(&void_inst, 0);
		}

		t_type type_system::t_int()
		{
			static detail::primitive_type_base int_inst("int");
			return t_type(&int_inst, 0);
		}

		t_type type_system::t_int_array(std::size_t rank)
		{
			return t_int().with_rank(rank);
		}

		t_type type_system::t_boolean()
		{
			static detail::primitive_type_base boolean_inst("boolean");
			return t_type(&boolean_inst, 0);
		}

		t_type type_system::t_boolean_array(std::size_t rank)
		{
			return t_boolean().with_rank(rank);
		}

		namespace detail
		{
			void extract_typesystem(const ast::program& prog, type_system& ts, const t_type& string_type)
			{
				std::vector<class_def*> classes{};

				for(auto&& clazz: prog.classes())
				{
					class_def* def = ts.new_class(clazz->name(), clazz.get());
					classes.push_back(def);
				}

				for(auto&& clazz_def: classes)
				{
					member_collector collector(ts, *clazz_def, string_type);
					collector.do_visit(clazz_def->decl());
				}
			}
		}
	}
}
