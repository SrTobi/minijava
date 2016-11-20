#include "symbol_def.hpp"

#include "exceptions.hpp"
#include "semantic/semantic_error.hpp"

namespace minijava
{
	namespace semantic
	{
		field_def::field_def(const symbol& name, const t_type& type, const class_def& clazz, const ast::var_decl* decl)
			: _name(name)
			, _type(type)
			, _class(clazz)
			, _decl(decl)
		{
			assert(!_decl || name == decl->name());
		}

		const ast::var_decl& field_def::decl() const
		{
			assert(!is_external());
			return *_decl;
		}

		symbol field_def::name() const
		{
			return _name;
		}

		t_type field_def::type() const
		{
			return _type;
		}

		bool field_def::is_local() const
		{
			return false;
		}

		bool field_def::is_external() const
		{
			return _decl == nullptr;
		}

		const class_def& field_def::clazz() const
		{
			return _class;
		}


	    var_def::var_def(const symbol& name, const t_type& type, const method_def& method, const ast::var_decl* decl)
			: _name(name)
			, _type(type)
			, _method(method)
			, _decl(decl)
		{
			assert(!_decl || name == decl->name());
		}

		const ast::var_decl& var_def::decl() const
		{
			assert(!is_external());
			return *_decl;
		}

		symbol var_def::name() const
		{
			return _name;
		}

		t_type var_def::type() const
		{
			return _type;
		}

		bool var_def::is_local() const
		{
			return true;
		}

		bool var_def::is_external() const
		{
			return _decl == nullptr;
		}

		const class_def& var_def::clazz() const
		{
			return method().clazz();
		}

		const method_def& var_def::method() const
		{
			return _method;
		}

		global_def::global_def(const symbol& name, const t_type& type)
			: _name(name)
			, _type(type)
		{
		}

		const ast::var_decl& global_def::decl() const
		{
			MINIJAVA_NOT_REACHED();
		}

		symbol global_def::name() const
		{
			return _name;
		}

		t_type global_def::type() const
		{
			return _type;
		}

		bool global_def::is_local() const
		{
			return false;
		}

		bool global_def::is_external() const
		{
			return true;
		}

		method_def::method_def(const symbol& name, const t_type& ret_type, bool is_static, const class_def& clazz, const ast::method* decl, def_annotations& def_a)
			: _name(name)
			, _ret_type(ret_type)
			, _static(is_static)
			, _class(clazz)
			, _decl(decl)
			, _def_a(def_a)
		{
			assert(!_decl || name == decl->name());
		}

		const ast::method& method_def::decl() const
		{
			assert(!is_external());
			return *_decl;
		}

		symbol method_def::name() const
		{
			return _name;
		}

		t_type method_def::type() const
		{
			return _ret_type;
		}

		bool method_def::is_local() const
		{
			return false;
		}

		bool method_def::is_external() const
		{
			return _decl == nullptr;
		}

		bool method_def::is_static() const
		{
			return _static;
		}

		const class_def& method_def::clazz() const
		{
			return _class;
		}

		const std::vector<const var_def*> method_def::parameters() const
		{
			return _parameters;
		}

		var_def* method_def::add_parameter(const t_type& type, const symbol& name, const ast::var_decl* decl)
		{
			if (!type.is_instantiable()) {
				throw semantic_error("Type " + type.to_string() + " can not be used as parameter");
			}
			auto param = std::make_unique<var_def>(name, type, *this, decl);
			auto ptr = param.get();
			_parameters.push_back(ptr);
			_def_a.store(std::move(param));
			return ptr;
		}


		class_def::class_def(const symbol& name, const ast::class_declaration* decl, def_annotations& def_a)
			: _name(name)
			, _decl(decl)
			, _def_a(def_a)
		{
			assert(!_decl || name == decl->name());
		}


		const ast::class_declaration& class_def::decl() const
		{
			assert(!is_external());
			return *_decl;
		}

		symbol class_def::name() const
		{
			return _name;
		}

		bool class_def::is_local() const
		{
			return false;
		}

		bool class_def::is_external() const
		{
			return _decl == nullptr;
		}

		const method_def* class_def::method(const symbol& name) const
		{
			auto it = _methods.find(name);
			if (it != _methods.end()) {
				return it->second;
			}
			return nullptr;
		}

		const field_def* class_def::field(const symbol& name) const
		{
			auto it = _fields.find(name);
			if (it != _fields.end()) {
				return it->second;
			}
			return nullptr;
		}

		const std::unordered_map<symbol, const method_def*> class_def::methods() const noexcept
		{
			return _methods;
		}

		const std::unordered_map<symbol, const field_def*> class_def::fields() const noexcept
		{
			return _fields;
		}

		field_def* class_def::new_field(const t_type& type, const symbol& name, const ast::var_decl* decl)
		{
			using namespace std::string_literals;
			if (!type.is_instantiable()) {
				throw semantic_error("Type " + type.to_string() + " can not be used as field");
			}
			auto field = std::make_unique<field_def>(name, type, *this, decl);
			bool inserted;
			std::tie(std::ignore, inserted) = _fields.emplace(name, field.get());
			if (!inserted) {
				throw semantic_error("Field with name '"s + name.c_str() + "' has already been defined in '" + this->name().c_str() + "'!");
			}
			return _def_a.store(std::move(field));
		}

		method_def* class_def::new_method(const t_type& ret_type, const symbol& name, const ast::method* decl, bool is_static)
		{
			using namespace std::string_literals;
			auto method = std::make_unique<method_def>(name, ret_type, is_static, *this, decl, _def_a);
			if (!is_static) {
				bool inserted;
				std::tie(std::ignore, inserted) = _methods.emplace(name, method.get());
				if (!inserted) {
					throw semantic_error("Method with name '"s + name.c_str() + "' has already been defined in '" + this->name().c_str() + "'!");
				}
			}
			return _def_a.store(std::move(method));
		}


		const class_def& def_annotations::operator[](const ast::class_declaration& decl) const
		{
			return def_of(decl);
		}

		const method_def& def_annotations::operator[](const ast::method& decl) const
		{
			return def_of(decl);
		}

		const symbol_def& def_annotations::operator[](const ast::var_decl& decl) const
		{
			return def_of(decl);
		}


		const class_def& def_annotations::def_of(const ast::class_declaration& decl) const
		{
			assert(_definitions.count(&decl));
			return static_cast<const class_def&>(*_definitions.at(&decl));
		}

		const method_def& def_annotations::def_of(const ast::method& decl) const
		{
			assert(_definitions.count(&decl));
			return static_cast<const method_def&>(*_definitions.at(&decl));
		}

		const symbol_def& def_annotations::def_of(const ast::var_decl& decl) const
		{
			assert(_definitions.count(&decl));
			return static_cast<const method_def&>(*_definitions.at(&decl));
		}

		void def_annotations::_store(std::unique_ptr<const symbol_def> def)
		{
			if (def->is_external()) {
				_externals.push_back(std::move(def));
			} else {
				assert(!_definitions.count(&def->decl()));
				_definitions.emplace(&def->decl(), std::move(def));
			}
		}
	}
}
