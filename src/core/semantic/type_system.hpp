#pragma once

#include <unordered_map>
#include <vector>

#include "symbol/symbol.hpp"
#include "parser/ast.hpp"

namespace minijava
{
	namespace semantic
	{
		class symbol_def;
		class field_def;
		class var_def;
		class method_def;
		class class_def;
		class def_annotations;

		namespace detail {
			struct type_base;
		}

		class t_type
		{
		public:
			t_type(const detail::type_base* inner, std::size_t rank);

			std::size_t rank() const noexcept;
			bool is_reference() const noexcept;
			bool is_array() const noexcept;
			bool is_objref() const noexcept;
			bool has_member() const noexcept;
			bool is_instantiable() const noexcept;
			t_type pure() const noexcept;
			t_type subrank() const;
			t_type with_rank(std::size_t rank) const;
			const class_def& objref() const;
			std::string to_string() const;

			friend bool operator==(const t_type& lhs, const t_type& rhs);
			friend bool operator!=(const t_type& lhs, const t_type& rhs);


		private:
			const detail::type_base* _inner;
			std::size_t _rank;
		};


		class type_system
		{
		public:
			struct type_system_builder;
		public:
			type_system(def_annotations& def_a);

			t_type resolve(const ast::type& type) const;
			t_type resolve(const symbol& name) const;
			const class_def* resolve_class(const symbol& name) const;
			bool is_assignable(const t_type& from, const t_type& to) const;

			const std::unordered_map<symbol, const class_def*>& classes() const;

			class_def* new_class(const symbol& name, const ast::class_declaration* decl);

			static t_type t_null();
			static t_type t_void();
			static t_type t_int();
			static t_type t_int_array(std::size_t rank);
			static t_type t_boolean();
			static t_type t_boolean_array(std::size_t rank);
		private:
			def_annotations& _def_a;
			std::unordered_map<symbol, const class_def*> _classes;
		};

		type_system extract_typesystem(const ast::program& prog, def_annotations& def_a);
	}
}
