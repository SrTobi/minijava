#ifndef MINIJAVA_INCLUDED_FROM_PARSER_PARSER_HPP
#error "Never `#include <parser/parser.tpp>` directly; `#include <parser/parser.hpp>` instead."
#endif

#include <array>
#include <initializer_list>
#include <algorithm>
#include <boost/algorithm/string/join.hpp>

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

            void parse_field()
            {

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

            void parse_method()
            {

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

            void parse_basic_type()
            {

            }

            void parse_statement()
            {

            }

            static constexpr auto block_first = set_t<
                token_type::left_brace
            >{};

            void parse_block()
            {
                assert(current_is(token_type::left_brace));
                advance();
                consume(token_type::right_brace);
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

            template<token_type... TTs>
            bool current_is(token_type_set<TTs...>)
            {
                return current_is(TTs...);
            }

            template<typename... TTs>
            bool current_is(const TTs... tts)
            {
                const auto list = std::initializer_list<token_type>{ tts...};
                return std::find(list.begin(), list.end(), current().type()) != list.end();
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
