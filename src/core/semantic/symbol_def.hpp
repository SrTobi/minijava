#pragma once

#include <unordered_map>
#include <vector>

#include "parser/ast.hpp"
#include "semantic/type_system.hpp"

namespace minijava
{
	namespace semantic
	{
		class t_type;
		class type_system;
		class symbol_def;
		class field_def;
		class var_def;
		class method_def;
		class class_def;
		class def_annotations;

		class symbol_def
		{
		public:
			virtual ~symbol_def() = default;

			virtual const ast::node& decl() const = 0;
			virtual symbol name() const = 0;
			virtual t_type type() const = 0;
			virtual bool is_local() const = 0;
			virtual bool is_external() const = 0;
		};

		class field_def : public symbol_def
		{
		public:
			field_def(const symbol& name, const t_type& type, const class_def& clazz, const ast::var_decl* decl);

			virtual const ast::var_decl& decl() const override;
			virtual symbol name() const override;
			virtual t_type type() const override;
			virtual bool is_local() const override;
			virtual bool is_external() const override;
			const class_def& clazz() const;

		private:
			symbol _name;
			t_type _type;
			const class_def& _class;
			const ast::var_decl* _decl;
		};

		class var_def : public symbol_def
		{
		public:
			var_def(const symbol& name, const t_type& type, const method_def& method, const ast::var_decl* decl);
			virtual const ast::var_decl& decl() const override;
			virtual symbol name() const override;
			virtual t_type type() const override;
			virtual bool is_local() const override;
			virtual bool is_external() const override;
			const class_def& clazz() const;
			const method_def& method() const;

		private:
			symbol _name;
			t_type _type;
			const method_def& _method;
			const ast::var_decl* _decl;
		};

		class global_def : public symbol_def
		{
		public:
			global_def(const symbol& name, const t_type& type);
			virtual const ast::var_decl& decl() const override;
			virtual symbol name() const override;
			virtual t_type type() const override;
			virtual bool is_local() const override;
			virtual bool is_external() const override;

		private:
			symbol _name;
			t_type _type;
		};

		class method_def : public symbol_def
		{
		public:
			method_def(const symbol& name, const t_type& ret_type, bool is_static, const class_def& clazz, const ast::method* decl, def_annotations& def_a);
			virtual const ast::method& decl() const override;
			virtual symbol name() const override;
			virtual t_type type() const override;
			virtual bool is_local() const override;
			virtual bool is_external() const override;
			virtual bool is_static() const;
			const class_def& clazz() const;
			const std::vector<const var_def*> parameters() const;

			var_def* add_parameter(const t_type& type, const symbol& name, const ast::var_decl* decl);
		private:
			std::vector<const var_def*> _parameters;
			symbol _name;
			t_type _ret_type;
			bool _static;
			const class_def& _class;
			const ast::method* _decl;
			def_annotations& _def_a;
		};

		class class_def : public symbol_def
		{
		public:
			class_def(const symbol& name, const ast::class_declaration* decl, def_annotations& def_a);
			virtual const ast::class_declaration& decl() const override;
			virtual symbol name() const override;
			virtual t_type type() const override = 0;
			virtual bool is_local() const override;
			virtual bool is_external() const override;

			const method_def* method(const symbol& name) const;
			const field_def* field(const symbol& name) const;

			const std::unordered_map<symbol, const method_def*> methods() const noexcept;
			const std::unordered_map<symbol, const field_def*> fields() const noexcept;

			field_def* new_field(const t_type& type, const symbol& name, const ast::var_decl* decl);
			method_def* new_method(const t_type& ret_type, const symbol& name, const ast::method* decl, bool static_ = false);

		private:
			std::unordered_map<symbol, const method_def*> _methods;
			std::unordered_map<symbol, const field_def*> _fields;
			symbol _name;
			const ast::class_declaration* _decl;
			def_annotations& _def_a;
		};

		class def_annotations
		{
		public:
			const class_def& operator[](const ast::class_declaration& decl) const;
			const method_def& operator[](const ast::method& decl) const;
			const symbol_def& operator[](const ast::var_decl& decl) const;

			const class_def& def_of(const ast::class_declaration& decl) const;
			const method_def& def_of(const ast::method& decl) const;
			const symbol_def& def_of(const ast::var_decl& decl) const;

			template<typename T>
			T* store(std::unique_ptr<T> def)
			{
				auto* ptr = def.get();
				_store(std::move(def));
				return ptr;
			}
		private:
			void _store(std::unique_ptr<const symbol_def> def);

		private:
			std::vector<std::unique_ptr<const symbol_def>> _externals;
			std::unordered_map<const ast::node*, std::unique_ptr<const symbol_def>> _definitions;
		};
	}
}
