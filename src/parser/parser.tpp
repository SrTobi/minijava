#ifndef MINIJAVA_INCLUDED_FROM_PARSER_PARSER_HPP
#error "Never `#include <parser/parser.tpp>` directly; `#include <parser/parser.hpp>` instead."
#endif

#include <array>
#include <initializer_list>
#include <algorithm>
#include <stack>
#include <boost/algorithm/string/join.hpp>

#include "exceptions.hpp"
#include "lexer/token_type.hpp"
#include "lexer/token.hpp"

namespace minijava
{
    namespace detail
    {
        template<token_type... TTs>
        struct token_type_set
        {
        };

        template<typename T1, typename T2>
        struct token_type_set_cat;

        template<token_type... TT1s, token_type... TT2s>
        struct token_type_set_cat<token_type_set<TT1s...>, token_type_set<TT2s...>>
        {
            using type = token_type_set<TT1s..., TT2s...>;
        };

        template<typename T1, typename T2>
        constexpr auto cat(T1, T2) -> typename token_type_set_cat<T1, T2>::type
        {
            using result_t = typename token_type_set_cat<T1, T2>::type;
            return result_t{};
        }

        constexpr bool is_binary_op(const token_type tt)
        {
            //TODO: implement it correctly
            return tt == token_type::plus;
        }

        constexpr int precedence(const token_type tt)
        {
            //TODO: implement it correctly
            assert(is_binary_op(tt));
            return 1;
        }

        constexpr bool is_left_assoc(const token_type tt)
        {
            return tt != token_type::assign;
        }

        template<typename InIterT>
        struct parser
        {
            template<token_type... TTs>
            using set_t = token_type_set<TTs...>;

            parser(InIterT first, InIterT /*last*/)
                : it(first)
            {
            }

            void parse_program()
            {
                while(!current_is(token_type::eof))
                {
                    expect(token_type::kw_class);
                    parse_class_declaration();
                }
            }

            void parse_class_declaration()
            {
                assert(current_is(token_type::kw_class));
                advance();
                consume(token_type::identifier);
                consume(token_type::left_brace);

                while(!current_is(token_type::right_brace))
                {
                    expect(token_type::kw_public);
                    parse_class_member();
                }
                consume(token_type::right_brace);
            }

            void parse_class_member()
            {
                assert(current_is(token_type::kw_public));
                advance();


                expect(cat(set_t<token_type::kw_static>{}, type_first));

                if(current_is(token_type::kw_static))
                {
                    return parse_main_method();
                }

                // parse field or method
                parse_type();
                consume(token_type::identifier);

                if(consume(token_type::semicolon, token_type::left_paren) == token_type::left_paren)
                {
                    // parse method
                    if(current_is(parameters_first))
                        parse_parameters();
                    consume(token_type::right_paren);
                    expect(token_type::left_brace);
                    parse_block();
                }
            }

            void parse_main_method()
            {
                using namespace std::string_literals;

                assert(current_is(token_type::kw_static));
                advance();
                consume(token_type::kw_void);
                consume(token_type::identifier);
                consume(token_type::left_paren);

                expect(token_type::identifier);
                if(std::strcmp(current().lexval().c_str(), "String") != 0)
                {
                    throw syntax_error(*it, "Expected 'String', but found "s + current().lexval().c_str());
                }
                advance();

                consume(token_type::left_bracket);
                consume(token_type::right_bracket);
                consume(token_type::identifier);
                consume(token_type::right_paren);
                expect(block_first);
                return parse_block();
            }

            static constexpr auto parameters_first = set_t<
                /* type_first */
                token_type::kw_int,
                token_type::kw_boolean,
                token_type::kw_void,
                token_type::identifier
            >{};

            static constexpr auto parameters_follow = set_t<
                token_type::right_paren
            >{};

            void parse_parameters()
            {
                assert(current_is(parameters_first));
                while (true) {
                    parse_parameter();
                    if(!current_is(token_type::comma))
                        return;
                    advance();
                    expect(type_first);
                }
            }

            void parse_parameter()
            {
                assert(current_is(type_first));
                parse_type();
                consume(token_type::identifier);
            }

            static constexpr auto type_first = set_t<
                token_type::kw_int,
                token_type::kw_boolean,
                token_type::kw_void,
                token_type::identifier
            >{};

            void parse_type()
            {
                assert(current_is(type_first));
                advance();
                while(current_is(token_type::left_bracket))
                {
                    advance();
                    consume(token_type::right_bracket);
                }
            }

            static constexpr auto block_first = set_t<
                token_type::left_brace
            >{};

            void parse_block()
            {
                assert(current_is(token_type::left_brace));
                advance();

                while(!current_is(token_type::right_brace))
                {
                    parse_block_statement();
                }
                advance();
            }

            void parse_block_statement()
            {
                parse_statement();
            }

            void parse_statement()
            {
                switch(current_type())
                {
                case token_type::left_brace:
                    return parse_block();
                case token_type::semicolon:
                    return parse_empty_statement();
                case token_type::kw_if:
                    return parse_if();
                default:
                    return parse_expression_statement();
                case token_type::kw_while:
                    return parse_while();
                case token_type::kw_return:
                    return parse_return();
                }
            }

            void parse_empty_statement()
            {
                assert(current_is(token_type::semicolon));
                advance();
            }

            void parse_while()
            {
                assert(current_is(token_type::kw_while));
                advance();

                consume(token_type::left_paren);
                parse_expression();
                consume(token_type::right_paren);

                parse_statement();
            }

            void parse_if()
            {
                assert(current_is(token_type::kw_if));
                advance();

                consume(token_type::left_paren);
                parse_expression();
                consume(token_type::right_paren);

                parse_statement();

                if(current_is(token_type::kw_else))
                {
                    advance();
                    parse_statement();
                }
            }

            void parse_expression_statement()
            {
                parse_expression();
                consume(token_type::semicolon);
            }

            void parse_return()
            {
                assert(current_is(token_type::kw_return));
                advance();

                if(!current_is(token_type::semicolon))
                {
                    parse_expression();
                }
                consume(token_type::semicolon);
            }

            static constexpr auto prefix_ops_first = set_t<
                token_type::logical_not,
                token_type::minus
            >{};

            static constexpr auto postfix_ops_first = set_t<
                token_type::dot,
                token_type::left_bracket
            >{};

            void parse_expression()
            {
                int min_prec = 0;
                std::stack<int> prec_stack{};
                std::stack<token_type> preop_stack{};
                //t = first token
            outer:
                assert(preop_stack.empty());
                while(current_is(prefix_ops_first))
                {
                    preop_stack.push(current_type());
                    advance();
                }

                expect(primary_expr_first);
                parse_primary();

                while(current_is(postfix_ops_first))
                {
                    parse_postfix_op();
                }

                while(!preop_stack.empty())
                {
                    preop_stack.pop();
                }

            inner:

                if (is_binary_op(current_type()) && precedence(current_type()) >= min_prec)
                {
                    int prec = precedence(current_type());
                    if (is_left_assoc(current_type()))
                    {
                        ++prec;
                    }

                    prec_stack.push(min_prec);

                    min_prec = prec;

                    advance();
                    goto outer;
                }

                if(!prec_stack.empty())
                {
                    min_prec = prec_stack.top();
                    prec_stack.pop();
                    goto inner;
                }
            }

            void parse_postfix_op()
            {
                assert(current_is(postfix_ops_first));
                if(current_is(token_type::left_bracket))
                {
                    // array access
                    advance();
                    parse_expression();
                    consume(token_type::right_bracket);
                }else{
                    // field access or method invocation
                    consume(token_type::dot);
                    consume(token_type::identifier);

                    if(current_is(token_type::left_paren))
                    {
                        // method invocation
                        advance();
                        parse_arguments();
                        consume(token_type::right_paren);
                    }
                }
            }

            static constexpr auto primary_expr_first = set_t<
                token_type::kw_null,
                token_type::kw_false,
                token_type::kw_true,
                token_type::integer_literal,
                token_type::identifier,
                token_type::kw_this,
                token_type::left_paren,
                token_type::kw_new
            >{};

            void parse_primary()
            {
                assert(current_is(primary_expr_first));

                switch(current_type())
                {
                case token_type::kw_null:
                case token_type::kw_false:
                case token_type::kw_true:
                case token_type::integer_literal:
                case token_type::kw_this:
                    advance();
                    return;
                case token_type::identifier:
                    advance();
                    if(current_is(token_type::left_paren))
                    {
                        advance();
                        parse_arguments();
                        consume(token_type::right_paren);
                    }
                    return;
                case token_type::left_paren:
                    advance();
                    parse_expression();
                    consume(token_type::right_paren);
                    return;
                case token_type::kw_new:
                    return parse_new_expression();
                default:
                    throw MINIJAVA_MAKE_ICE_MSG("Unexpected token");
                }
            }

            void parse_arguments()
            {
                if(current_is(token_type::right_paren))
                {
                    return;
                }
                while (true) {
                    parse_expression();
                    if(!current_is(token_type::comma))
                        return;
                    advance();
                }
            }

            void parse_new_expression()
            {
                assert(current_is(token_type::kw_new));
                throw minijava::not_implemented_error{};
            }

            template<token_type... TTs>
            token_type expect(token_type_set<TTs...>)
            {
                return expect(TTs...);
            }

            template<typename... TTs>
            token_type expect(const TTs... tts)
            {
                using namespace std::string_literals;
                if(!current_is(tts...))
                {
                    std::array<std::string, sizeof...(tts)> exp_toks = {{name(tts)...}};
                    throw syntax_error(*it, "Expected any of "s + boost::algorithm::join(exp_toks, ", "s) + " but found " + name(it->type()));
                }
                return it->type();
            }

            template<typename... TTs>
            token_type consume(const TTs... tts)
            {
                const auto tt = expect(tts...);
                advance();
                return tt;
            }

            token next()
            {
                advance();
                return *it;
            }

            void advance()
            {
                ++it;
            }

            token current()
            {
                return *it;
            }

            token_type current_type() noexcept
            {
                return current().type();
            }

            template<token_type... TTs>
            bool current_is(token_type_set<TTs...>)
            {
                return current_is(TTs...);
            }

            template<typename... TTs>
            bool current_is(const TTs... tts)
            {
                const auto list = std::initializer_list<token_type>{ tts...};
                return std::find(list.begin(), list.end(), current_type()) != list.end();
            }


            InIterT it;
        };
    }

    template<typename InIterT>
    void parse_program(InIterT first, InIterT last)
    {
        auto parser = detail::parser<InIterT>(first, last);
        return parser.parse_program();
    }
}
