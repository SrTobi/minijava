#pragma once

#include <string>
#include <stdexcept>

#include "lexer/token.hpp"

namespace minijava
{
    struct syntax_error: std::runtime_error
    {
		syntax_error() : std::runtime_error{"invalid syntax"} {}
		syntax_error(std::string msg) : std::runtime_error{std::move(msg)} {}
		syntax_error(const token& tok, const std::string& msg)
            : std::runtime_error{msg}
            , _line(tok.line())
            , _column(tok.column())
        {}

        std::size_t line() const noexcept
        {
            return _line;
        }

        std::size_t column() const noexcept
        {
            return _column;
        }
    private:
        std::size_t _line{};
        std::size_t _column{};
    };

    template<typename InIterT>
    void parse_program(InIterT first, InIterT last);
}


#define MINIJAVA_INCLUDED_FROM_PARSER_PARSER_HPP
#include "parser/parser.tpp"
#undef MINIJAVA_INCLUDED_FROM_PARSER_PARSER_HPP


/*

stack s (prec, expr, operator) = [];


parse_exprs()
{
    min_prec = 0
    t = first token
outer:
    stack preop-stack : unop = [];
    while(t is pre-op)
    {
        preop-stack.push(t.operator)
        t <- next token advance
    }

    result = parse_primary()
    t <- next token advance

    while(t is post-op)
    {
        result = t.operator(rusult)
    }

    while(!preop-stack.empty)
    {
        preop <- preop-stack.pop
        result = preop(result)
    }


inner:
    if (t is bin-op) && (t.prec >= min_prec)
    {
        prec = t.prec
        if (t.assoc == left)
        {
            ++prec
        }

        s.push(min_prec, result, t.operator)

        min_prec = prec

        t <- next token advance
        goto outer;
    }

    if(!s.empty)
    {
        (min_prec, lhs, op) <- s.pop
        result <- op(lhs, result)
        goto inner
    }

    return result
}

*/
