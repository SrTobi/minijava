#include "parser/serializer.hpp"

#include <iostream>


namespace minijava
{

	namespace detail
	{

		char space_between(const token_type lhs, const token_type rhs)
		{
			if (category(lhs) == token_category::synthetic) {
				return 0;
			}
			if (category(rhs) == token_category::synthetic) {
				return 0;
			}
			switch (lhs) {
			case token_type::bit_not:
			case token_type::decrement:
			case token_type::increment:
			case token_type::left_bracket:
			case token_type::left_paren:
			case token_type::logical_not:
				return 0;
			case token_type::left_brace:
			case token_type::semicolon:
				return '\n';
			default:
				break;
			}
			switch (rhs) {
			case token_type::comma:
			case token_type::decrement:
			case token_type::increment:
			case token_type::left_bracket:
			case token_type::right_bracket:
			case token_type::right_paren:
			case token_type::semicolon:
				return 0;
			default:
				break;
			}
			if ((rhs == token_type::left_paren) && (lhs == token_type::identifier)) {
				return 0;
			}
			if (lhs == token_type::right_brace) {
				if (rhs == token_type::kw_else) {
					return ' ';
				}
				return '\n';
			}
			return ' ';
		}

		int new_indent(const int old, const token_type tt)
		{
			switch (tt) {
			case token_type::left_brace:
				return old + 1;
			case token_type::right_brace:
				return old - 1;
			default:
				return old;
			}
		}


		int serialize_next_token(std::ostream& os,
								 const token& tok,
								 const int prev_indent,
								 const token_type prev_type)
		{
			const auto indent = new_indent(prev_indent, tok.type());
			if (const auto s = space_between(prev_type, tok.type())) {
				os << s;
				if (s == '\n') {
					const auto amount = (tok.type() == token_type::right_brace)
						? indent
						: prev_indent;
					for (auto i = 0; i < amount; ++i) {
						os << '\t';
					}
				}
			}
			switch (category(tok.type())) {
			case token_category::identifier:
			case token_category::literal:
				os << tok.lexval();
				break;
			case token_category::keyword:
			case token_category::punctuation:
				os << name(tok.type());
				break;
			case token_category::synthetic:
				break;
			}
			return indent;
		}

	}  // namespace detail

}  // namespace minijava
