#ifndef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_TYPE_HPP
#error "Never `#include <lexer/token_type.tpp>` directly; `#include <token_type.hpp>` instead."
#endif


namespace minijava
{

	constexpr const char * fancy_name(const token_type tt) noexcept
	{
		// TODO @Moritz Klammler: Complete this list.
		switch (tt) {
		case token_type::identifier: return "identifier";
		case token_type::integer_literal: return "integer literal";
		case token_type::kw_if: return "if";
		case token_type::kw_else: return "else";
		case token_type::left_paren: return "(";
		case token_type::right_paren: return ")";
		case token_type::eof: return "EOF";
		case token_type::unknown: return "UNKNOWN";
			// ...
		}
		return nullptr;
	}

}  // namespace minijava
