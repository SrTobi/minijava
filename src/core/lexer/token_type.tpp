#ifndef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_TYPE_HPP
#error "Never `#include <lexer/token_type.tpp>` directly; `#include <token_type.hpp>` instead."
#endif

#include <type_traits>
#include <utility>


namespace minijava
{

	namespace detail
	{

		// NB: Never ever make ODR-use of this array or the world will catch
		// fire and explode.  To be on the safe-side, only use it in constexpr
		// functions.  For run-time stuff, a copy should be made.

		constexpr std::pair<token_type, const char*> token_type_info_table[] = {
			{token_type::identifier,                   "identifier"},
			{token_type::integer_literal,              "integer literal"},
			{token_type::kw_abstract,                  "abstract"},
			{token_type::kw_assert,                    "assert"},
			{token_type::kw_boolean,                   "boolean"},
			{token_type::kw_break,                     "break"},
			{token_type::kw_byte,                      "byte"},
			{token_type::kw_case,                      "case"},
			{token_type::kw_catch,                     "catch"},
			{token_type::kw_char,                      "char"},
			{token_type::kw_class,                     "class"},
			{token_type::kw_const,                     "const"},
			{token_type::kw_continue,                  "continue"},
			{token_type::kw_default,                   "default"},
			{token_type::kw_double,                    "double"},
			{token_type::kw_do,                        "do"},
			{token_type::kw_else,                      "else"},
			{token_type::kw_enum,                      "enum"},
			{token_type::kw_extends,                   "extends"},
			{token_type::kw_false,                     "false"},
			{token_type::kw_finally,                   "finally"},
			{token_type::kw_final,                     "final"},
			{token_type::kw_float,                     "float"},
			{token_type::kw_for,                       "for"},
			{token_type::kw_goto,                      "goto"},
			{token_type::kw_if,                        "if"},
			{token_type::kw_implements,                "implements"},
			{token_type::kw_import,                    "import"},
			{token_type::kw_instanceof,                "instanceof"},
			{token_type::kw_interface,                 "interface"},
			{token_type::kw_int,                       "int"},
			{token_type::kw_long,                      "long"},
			{token_type::kw_native,                    "native"},
			{token_type::kw_new,                       "new"},
			{token_type::kw_null,                      "null"},
			{token_type::kw_package,                   "package"},
			{token_type::kw_private,                   "private"},
			{token_type::kw_protected,                 "protected"},
			{token_type::kw_public,                    "public"},
			{token_type::kw_return,                    "return"},
			{token_type::kw_short,                     "short"},
			{token_type::kw_static,                    "static"},
			{token_type::kw_strictfp,                  "strictfp"},
			{token_type::kw_super,                     "super"},
			{token_type::kw_switch,                    "switch"},
			{token_type::kw_synchronized,              "synchronized"},
			{token_type::kw_this,                      "this"},
			{token_type::kw_throws,                    "throws"},
			{token_type::kw_throw,                     "throw"},
			{token_type::kw_transient,                 "transient"},
			{token_type::kw_true,                      "true"},
			{token_type::kw_try,                       "try"},
			{token_type::kw_void,                      "void"},
			{token_type::kw_volatile,                  "volatile"},
			{token_type::kw_while,                     "while"},
			{token_type::not_equal,                    "!="},
			{token_type::logical_not,                  "!"},
			{token_type::left_paren,                   "("},
			{token_type::right_paren,                  ")"},
			{token_type::multiply_assign,              "*="},
			{token_type::multiply,                     "*"},
			{token_type::increment,                    "++"},
			{token_type::plus_assign,                  "+="},
			{token_type::plus,                         "+"},
			{token_type::comma,                        ","},
			{token_type::minus_assign,                 "-="},
			{token_type::decrement,                    "--"},
			{token_type::minus,                        "-"},
			{token_type::dot,                          "."},
			{token_type::divides_assign,               "/="},
			{token_type::divide,                       "/"},
			{token_type::colon,                        ":"},
			{token_type::semicolon,                    ";"},
			{token_type::left_shift_assign,            "<<="},
			{token_type::left_shift,                   "<<"},
			{token_type::less_equal,                   "<="},
			{token_type::less_than,                    "<"},
			{token_type::equal,                        "=="},
			{token_type::assign,                       "="},
			{token_type::greater_equal,                ">="},
			{token_type::right_shift_assign,           ">>="},
			{token_type::right_shift,                  ">>"},
			{token_type::unsigned_right_shift_assign,  ">>>="},
			{token_type::unsigned_right_shift,         ">>>"},
			{token_type::greater_than,                 ">"},
			{token_type::conditional,                  "?"},
			{token_type::modulo_assign,                "%="},
			{token_type::modulo,                       "%"},
			{token_type::bit_and_assign,               "&="},
			{token_type::logical_and,                  "&&"},
			{token_type::bit_and,                      "&"},
			{token_type::left_bracket,                 "["},
			{token_type::right_bracket,                "]"},
			{token_type::bit_xor_assign,               "^="},
			{token_type::bit_xor,                      "^"},
			{token_type::left_brace,                   "{"},
			{token_type::right_brace,                  "}"},
			{token_type::bit_not,                      "~"},
			{token_type::bit_or_assign,                "|="},
			{token_type::logical_or,                   "||"},
			{token_type::bit_or,                       "|"},
			{token_type::eof,                          "EOF"},
		};

	}  // namespace detail


	constexpr token_category category(const token_type tt) noexcept
	{
		using raw_type_t = std::underlying_type_t<token_type>;
		using raw_cat_t = std::underlying_type_t<token_category>;
		static_assert(std::is_same<raw_type_t, raw_cat_t>{}, "");
		const auto bitmask = raw_cat_t{0xf000};
		const auto raw_type = static_cast<raw_type_t>(tt);
		const auto raw_cat = raw_cat_t((0U + raw_type) & (0U + bitmask));
		return static_cast<token_category>(raw_cat);
	}

	constexpr std::size_t index(const token_type tt) noexcept
	{
		const auto raw_type = static_cast<std::size_t>(tt);
		const auto raw_cat = static_cast<std::size_t>(category(tt));
		const auto idx = raw_type - raw_cat;
		if (idx < total_token_type_count) {
			if (detail::token_type_info_table[idx].first == tt) {
				return idx;
			}
		}
		return total_token_type_count;
	}

	constexpr token_type token_type_at_index(const std::size_t idx) noexcept
	{
		if (idx < total_token_type_count) {
			return detail::token_type_info_table[idx].first;
		}
		return token_type{};
	}

	constexpr const char * name(const token_type tt) noexcept
	{
		const auto idx = index(tt);
		return (idx < total_token_type_count)
			? detail::token_type_info_table[idx].second
			: nullptr;
	}


	constexpr const char * name(const token_category cat) noexcept
	{
		switch (cat)
		{
		case token_category::identifier:  return "identifier";
		case token_category::literal:     return "literal";
		case token_category::keyword:     return "keyword";
		case token_category::punctuation: return "punctuation";
		case token_category::synthetic:   return "synthetic";
		}
		return nullptr;
	}


}  // namespace minijava
