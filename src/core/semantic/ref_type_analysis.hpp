#pragma once

#include <unordered_map>
#include <memory>

#include "parser/ast.hpp"
#include "type_system.hpp"

namespace minijava
{
	namespace semantic
	{
		class type_annotation
		{
		public:
			type_annotation(std::unordered_map<const ast::expression*, t_type> annotations);
			t_type operator[](const ast::expression& node) const;

			const std::unordered_map<const ast::expression*, t_type>& types() const;
		private:
			std::unordered_map<const ast::expression*, t_type> _types;
		};

		class ref_annotation
		{
		public:
			ref_annotation(std::unordered_map<const ast::node*, const symbol_def*>);

			const method_def& operator[](const ast::method_invocation& node) const;
			const symbol_def& operator[](const ast::variable_access& node) const;

			const std::unordered_map<const ast::node*, const symbol_def*> refs() const;
		private:
			std::unordered_map<const ast::node*, const symbol_def*> _refs;
		};

		std::pair<type_annotation, ref_annotation> analyse_program(const ast::program& prog, const type_system& typesystem, def_annotations& def_a);
	}
}
