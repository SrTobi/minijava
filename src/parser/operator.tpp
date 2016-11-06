#ifndef MINIJAVA_INCLUDED_FROM_PARSER_PARSER_HPP
#error "Never `#include <parser/operator.tpp>` directly; `#include <parser/parser.hpp>` instead."
#endif

namespace minijava
{
    namespace detail
    {
        enum class op_assoc: unsigned char
        {
            none   = 0b00,
            left   = 0b01,
            right  = 0b10,
            either = 0b11
        };

        enum class op_arity: unsigned char
        {
            none   = 0b00,
            unary  = 0b01,
            binary = 0b10,
            either = 0b11
        };

        struct operator_info
        {
            token_type type;
            bool enabled;
            op_arity arity;
            int prec;
            op_assoc assoc;
        };

        constexpr operator_info op_infos[] = {
            {token_type::not_equal,                      true, op_arity::binary,  8, op_assoc::left},
            {token_type::logical_not,                    true, op_arity::unary,  13, op_assoc::right},
            {token_type::left_paren,                    false, op_arity::none,   -1, op_assoc::none},
            {token_type::right_paren,                   false, op_arity::none,   -1, op_assoc::none},
            {token_type::multiply_assign,               false, op_arity::binary,  1, op_assoc::right},
            {token_type::multiply,                       true, op_arity::binary, 12, op_assoc::left},
            {token_type::increment,                     false, op_arity::unary,  -1, op_assoc::either},
            {token_type::plus_assign,                   false, op_arity::binary,  1, op_assoc::right},
            {token_type::plus,                           true, op_arity::binary, 11, op_assoc::left},
            {token_type::comma,                         false, op_arity::none,   -1, op_assoc::none},
            {token_type::minus_assign,                  false, op_arity::binary,  1, op_assoc::right},
            {token_type::decrement,                     false, op_arity::unary,  -1, op_assoc::either},
            {token_type::minus,                          true, op_arity::binary, 11, op_assoc::left},
            {token_type::dot,                           false, op_arity::none,   -1, op_assoc::none},
            {token_type::divides_assign,                false, op_arity::binary,  1, op_assoc::right},
            {token_type::divides,                        true, op_arity::binary, 12, op_assoc::left},
            {token_type::colon,                         false, op_arity::none,    2, op_assoc::right},
            {token_type::semicolon,                     false, op_arity::none,   -1, op_assoc::none},
            {token_type::left_shift_assign,             false, op_arity::binary,  1, op_assoc::right},
            {token_type::left_shift,                    false, op_arity::binary, 10, op_assoc::left},
            {token_type::less_equal,                     true, op_arity::binary,  9, op_assoc::left},
            {token_type::less_than,                      true, op_arity::binary,  9, op_assoc::left},
            {token_type::equal,                          true, op_arity::binary,  8, op_assoc::left},
            {token_type::assign,                         true, op_arity::binary,  1, op_assoc::right},
            {token_type::greater_equal,                  true, op_arity::binary,  9, op_assoc::left},
            {token_type::right_shift_assign,            false, op_arity::binary,  1, op_assoc::right},
            {token_type::right_shift,                   false, op_arity::binary, 10, op_assoc::left},
            {token_type::unsigned_right_shift_assign,   false, op_arity::binary,  1, op_assoc::right},
            {token_type::unsigned_right_shift,          false, op_arity::binary, 10, op_assoc::left},
            {token_type::greater_than,                   true, op_arity::binary,  9, op_assoc::left},
            {token_type::conditional,                   false, op_arity::none,    2, op_assoc::right},
            {token_type::modulo_assign,                 false, op_arity::binary,  1, op_assoc::right},
            {token_type::modulo,                         true, op_arity::binary, 12, op_assoc::left},
            {token_type::bit_and_assign,                false, op_arity::binary,  1, op_assoc::right},
            {token_type::logical_and,                    true, op_arity::binary,  4, op_assoc::left},
            {token_type::bit_and,                       false, op_arity::binary,  7, op_assoc::left},
            {token_type::left_bracket,                  false, op_arity::none,   -1, op_assoc::none},
            {token_type::right_bracket,                 false, op_arity::none,   -1, op_assoc::none},
            {token_type::bit_xor_assign,                false, op_arity::binary,  1, op_assoc::right},
            {token_type::bit_xor,                       false, op_arity::binary,  6, op_assoc::left},
            {token_type::left_brace,                    false, op_arity::none,   -1, op_assoc::none},
            {token_type::right_brace,                   false, op_arity::none,   -1, op_assoc::none},
            {token_type::bit_not,                       false, op_arity::unary,  13, op_assoc::right},
            {token_type::bit_or_assign,                 false, op_arity::binary,  1, op_assoc::right},
            {token_type::logical_or,                     true, op_arity::binary,  3, op_assoc::left},
            {token_type::bit_or,                        false, op_arity::binary,  5, op_assoc::left},
        };

        constexpr operator_info lookup_op_info(const token_type tt) noexcept
        {
            if(category(tt) != token_category::punctuation)
                return {tt, false, op_arity::none, -1, op_assoc::none};
            constexpr auto cat_begin = static_cast<std::size_t>(token_category::punctuation);
            const auto raw_tt = static_cast<std::size_t>(tt);
            return op_infos[raw_tt - cat_begin];
        }

        constexpr bool is_binary_op(const token_type tt) noexcept
        {
            const auto loi = lookup_op_info(tt);
            return loi.enabled && (loi.arity == op_arity::binary);
        }

        constexpr int precedence(const token_type tt) noexcept
        {
            return lookup_op_info(tt).prec;
        }

        constexpr bool is_left_assoc(const token_type tt) noexcept
        {
            return lookup_op_info(tt).assoc == op_assoc::left;
        }

    }
}
