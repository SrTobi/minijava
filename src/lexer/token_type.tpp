#ifndef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_TYPE_HPP
#error "Never `#include <lexer/token_type.tpp>` directly; `#include <token_type.hpp>` instead."
#endif


namespace minijava
{

	constexpr token_category category(const token_type tt) noexcept
	{
		const auto rawtt = static_cast<std::uint16_t>(tt);
		const auto rawcat = (rawtt >> 12);
		const auto cat = static_cast<token_category>(rawcat);
		return cat;
	}


	constexpr const char * fancy_name(const token_type tt) noexcept
	{
		switch (tt) {
		case token_type::identifier:                   return "identifier";
		case token_type::integer_literal:              return "integer literal";
		case token_type::kw_abstract:                  return "abstract";
		case token_type::kw_assert:                    return "assert";
		case token_type::kw_boolean:                   return "boolean";
		case token_type::kw_break:                     return "break";
		case token_type::kw_byte:                      return "byte";
		case token_type::kw_case:                      return "case";
		case token_type::kw_catch:                     return "catch";
		case token_type::kw_char:                      return "char";
		case token_type::kw_class:                     return "class";
		case token_type::kw_const:                     return "const";
		case token_type::kw_continue:                  return "continue";
		case token_type::kw_default:                   return "default";
		case token_type::kw_double:                    return "double";
		case token_type::kw_do:                        return "do";
		case token_type::kw_else:                      return "else";
		case token_type::kw_enum:                      return "enum";
		case token_type::kw_extends:                   return "extends";
		case token_type::kw_false:                     return "false";
		case token_type::kw_finally:                   return "finally";
		case token_type::kw_final:                     return "final";
		case token_type::kw_float:                     return "float";
		case token_type::kw_for:                       return "for";
		case token_type::kw_goto:                      return "goto";
		case token_type::kw_if:                        return "if";
		case token_type::kw_implements:                return "implements";
		case token_type::kw_import:                    return "import";
		case token_type::kw_instanceof:                return "instanceof";
		case token_type::kw_interface:                 return "interface";
		case token_type::kw_int:                       return "int";
		case token_type::kw_long:                      return "long";
		case token_type::kw_native:                    return "native";
		case token_type::kw_new:                       return "new";
		case token_type::kw_null:                      return "null";
		case token_type::kw_package:                   return "package";
		case token_type::kw_private:                   return "private";
		case token_type::kw_protected:                 return "protected";
		case token_type::kw_public:                    return "public";
		case token_type::kw_return:                    return "return";
		case token_type::kw_short:                     return "short";
		case token_type::kw_static:                    return "static";
		case token_type::kw_strictfp:                  return "strictfp";
		case token_type::kw_super:                     return "super";
		case token_type::kw_switch:                    return "switch";
		case token_type::kw_synchronized:              return "synchronized";
		case token_type::kw_this:                      return "this";
		case token_type::kw_throws:                    return "throws";
		case token_type::kw_throw:                     return "throw";
		case token_type::kw_transient:                 return "transient";
		case token_type::kw_true:                      return "true";
		case token_type::kw_try:                       return "try";
		case token_type::kw_void:                      return "void";
		case token_type::kw_volatile:                  return "volatile";
		case token_type::kw_while:                     return "while";
		case token_type::not_equal:                    return "!=";
		case token_type::logical_not:                  return "!";
		case token_type::left_paren:                   return "(";
		case token_type::right_paren:                  return ")";
		case token_type::multiply_assign:              return "*=";
		case token_type::multiply:                     return "*";
		case token_type::increment:                    return "++";
		case token_type::plus_assign:                  return "+=";
		case token_type::plus:                         return "+";
		case token_type::comma:                        return ",";
		case token_type::minus_assign:                 return "-=";
		case token_type::decrement:                    return "--";
		case token_type::minus:                        return "-";
		case token_type::dot:                          return ".";
		case token_type::divides_assign:               return "/=";
		case token_type::divides:                      return "/";
		case token_type::colon:                        return ":";
		case token_type::semicolon:                    return ";";
		case token_type::left_shift_assign:            return "<<=";
		case token_type::left_shift:                   return "<<";
		case token_type::less_equal:                   return "<=";
		case token_type::less_than:                    return "<";
		case token_type::equal:                        return "==";
		case token_type::assign:                       return "=";
		case token_type::greater_equal:                return ">=";
		case token_type::right_shift_assign:           return ">>=";
		case token_type::right_shift:                  return ">>";
		case token_type::unsigned_right_shift_assign:  return ">>=";
		case token_type::unsigned_right_shift:         return ">>>";
		case token_type::greater_than:                 return ">=";
		case token_type::conditional:                  return "?";
		case token_type::modulo_assign:                return "%=";
		case token_type::modulo:                       return "%";
		case token_type::bit_and_assign:               return "&=";
		case token_type::logical_and:                  return "&&";
		case token_type::bit_and:                      return "&";
		case token_type::left_bracket:                 return "[";
		case token_type::right_bracket:                return "]";
		case token_type::bit_xor_assign:               return "^=";
		case token_type::bit_xor:                      return "^";
		case token_type::left_brace:                   return "{";
		case token_type::right_brace:                  return "}";
		case token_type::bit_not:                      return "~";
		case token_type::bit_or_assign:                return "|=";
		case token_type::logical_or:                   return "||";
		case token_type::bit_or:                       return "|";
		case token_type::eof:                          return "EOF";
		}
		return nullptr;
	}

}  // namespace minijava
