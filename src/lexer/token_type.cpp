#include "token_type.hpp"

#include <ostream>


namespace minijava
{

	const std::array<token_type, 102>& all_token_types() noexcept
	{
		static const std::array<token_type, 102> values = {{
			token_type::identifier,
			token_type::integer_literal,
			token_type::kw_abstract,
			token_type::kw_assert,
			token_type::kw_boolean,
			token_type::kw_break,
			token_type::kw_byte,
			token_type::kw_case,
			token_type::kw_catch,
			token_type::kw_char,
			token_type::kw_class,
			token_type::kw_const,
			token_type::kw_continue,
			token_type::kw_default,
			token_type::kw_double,
			token_type::kw_do,
			token_type::kw_else,
			token_type::kw_enum,
			token_type::kw_extends,
			token_type::kw_false,
			token_type::kw_finally,
			token_type::kw_final,
			token_type::kw_float,
			token_type::kw_for,
			token_type::kw_goto,
			token_type::kw_if,
			token_type::kw_implements,
			token_type::kw_import,
			token_type::kw_instanceof,
			token_type::kw_interface,
			token_type::kw_int,
			token_type::kw_long,
			token_type::kw_native,
			token_type::kw_new,
			token_type::kw_null,
			token_type::kw_package,
			token_type::kw_private,
			token_type::kw_protected,
			token_type::kw_public,
			token_type::kw_return,
			token_type::kw_short,
			token_type::kw_static,
			token_type::kw_strictfp,
			token_type::kw_super,
			token_type::kw_switch,
			token_type::kw_synchronized,
			token_type::kw_this,
			token_type::kw_throws,
			token_type::kw_throw,
			token_type::kw_transient,
			token_type::kw_true,
			token_type::kw_try,
			token_type::kw_void,
			token_type::kw_volatile,
			token_type::kw_while,
			token_type::not_equal,
			token_type::logical_not,
			token_type::left_paren,
			token_type::right_paren,
			token_type::multiply_assign,
			token_type::multiply,
			token_type::increment,
			token_type::plus_assign,
			token_type::plus,
			token_type::comma,
			token_type::minus_assign,
			token_type::decrement,
			token_type::minus,
			token_type::dot,
			token_type::divides_assign,
			token_type::divides,
			token_type::colon,
			token_type::semicolon,
			token_type::left_shift_assign,
			token_type::left_shift,
			token_type::less_equal,
			token_type::less_than,
			token_type::equal,
			token_type::assign,
			token_type::greater_equal,
			token_type::right_shift_assign,
			token_type::right_shift,
			token_type::unsigned_right_shift_assign,
			token_type::unsigned_right_shift,
			token_type::greater_than,
			token_type::conditional,
			token_type::modulo_assign,
			token_type::modulo,
			token_type::bit_and_assign,
			token_type::logical_and,
			token_type::bit_and,
			token_type::left_bracket,
			token_type::right_bracket,
			token_type::bit_xor_assign,
			token_type::bit_xor,
			token_type::left_brace,
			token_type::right_brace,
			token_type::bit_not,
			token_type::bit_or_assign,
			token_type::logical_or,
			token_type::bit_or,
			token_type::eof,
		}};
		return values;
	}

	std::ostream& operator<<(std::ostream& os, const token_type tt)
	{
		if (const auto fancy = fancy_name(tt)) {
			os << fancy;
		} else {
			os << "invalid token type " << static_cast<int>(tt);
		}
		return os;
	}

}  // namespace minijava
