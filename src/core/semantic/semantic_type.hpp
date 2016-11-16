#pragma once

#include <string>

#include "symbol/symbol.hpp"
#include "parser/ast.hpp"
#include "exceptions.hpp"

namespace minijava
{
	enum class semantic_type_kind
	{
		kind_void,
		kind_boolean,
		kind_int,
		kind_reference,
		kind_null_reference
	};

	class semantic_type
	{
	public:
		semantic_type(const ast::class_declaration* decl, std::size_t rank = 0) noexcept
			: _decl(decl)
			, _rank(rank)
			, _kind(semantic_type_kind::kind_reference)
		{
			assert(decl);
		}

		semantic_type(semantic_type_kind kind, std::size_t rank = 0) noexcept
			: _rank(rank)
			, _kind(kind)
		{
			assert(kind != semantic_type_kind::kind_reference);
		}

		semantic_type_kind kind() const noexcept
		{
			return _kind;
		}

		std::size_t rank() const noexcept
		{
			return _rank;
		}

		bool is_array() const noexcept
		{
			return rank() > 0;
		}

		bool is_reference() const noexcept
		{
			return is_array()
				|| kind() == semantic_type_kind::kind_null_reference
				|| kind() == semantic_type_kind::kind_reference;
		}

		bool is_objref() const noexcept
		{
			return kind() == semantic_type_kind::kind_reference && !is_array();
		}

		semantic_type subrank() const noexcept
		{
			assert(is_array());
			semantic_type clone = *this;
			--clone._rank;
			return clone;
		}

		const ast::class_declaration* decl() const
		{
			assert(kind() == semantic_type_kind::kind_reference);
			return _decl;
		}

		std::string to_string() const
		{
			std::string result;
			switch(kind())
			{
			case semantic_type_kind::kind_null_reference: result = "null"; break;
			case semantic_type_kind::kind_reference: result = _decl->name().c_str(); break;
			case semantic_type_kind::kind_void: result = "void"; break;
			case semantic_type_kind::kind_int: result = "int"; break;
			case semantic_type_kind::kind_boolean: result = "boolean"; break;
			default:
				MINIJAVA_NOT_REACHED();
			}
			for(std::size_t i = 0; i < rank(); ++i)
			{
				result += "[]";
			}
			return result;
		}


		friend bool operator==(const semantic_type& lhs, const semantic_type& rhs) noexcept
		{
			return lhs.kind() == rhs.kind()
				&& lhs.rank() == rhs.rank()
				&& lhs._decl == rhs._decl;
		}


		friend bool operator!=(const semantic_type& lhs, const semantic_type& rhs) noexcept
		{
			return !(lhs == rhs);
		}


	private:
		const ast::class_declaration* _decl = nullptr;
		std::size_t _rank;
		semantic_type_kind _kind;
	};
}
