#include "parser/parser.hpp"
#include "lexer/lexer.hpp"
#include "symbol_pool.hpp"
#include <vector>
#include <utility>
#include <random>
#include <iostream>
#include <cstdlib>

using tt = minijava::token_type;
namespace {

    minijava::symbol_pool<> g_pool{};

    struct generator
    {
        static double get_expr_dist_param()
        {
            static const double v = [](){
                if(const auto envp = std::getenv("EXPRDIST"))
                    return std::stod(envp);
                return 0.1;
            }();
            return v;
        }

        generator()
        {
            std::random_device rnddev{};
            engine.seed(rnddev());
        }

        void gen_program()
        {
            auto dist = std::exponential_distribution<double>{0.5};
            auto n_classes = static_cast<int>(dist(engine)+0.5);

            while(n_classes--)
            {
                gen_class_decl();
            }

            tokens.push_back(minijava::token::create(tt::eof));
        }

        void gen_class_decl()
        {
            push(tt::kw_class);
            push_Id();
            push(tt::left_brace);

            auto dist = std::exponential_distribution<double>{0.7};
            auto n_members = static_cast<int>(dist(engine)+0.5);

            while(n_members--)
            {
                gen_class_member();
            }

            push(tt::right_brace);
        }

        void gen_class_member()
        {
            std::uniform_int_distribution<int> dist(0,2);

            switch(dist(engine))
            {
            case 0: return gen_field();
            case 1: return gen_method();
            case 2: return gen_main_method();
            }
            throw 3;
        }

        void gen_field()
        {
            push(tt::kw_public);
            gen_type();
            push_id();
            push(tt::semicolon);
        }

        void gen_main_method()
        {
            push(tt::kw_public);
            push(tt::kw_static);
            push(tt::kw_void);
            push_id();
            push(tt::left_paren);
            push_id("String");
            push(tt::left_bracket);
            push(tt::right_bracket);
            push_id();
            push(tt::right_paren);
            gen_block();
        }

        void gen_method()
        {
            std::bernoulli_distribution dist(0.8);
            push(tt::kw_public);
            gen_type();
            push_id();
            push(tt::left_paren);
            if(dist(engine))
                gen_parameters();
            push(tt::right_paren);
            gen_block();
        }

        void gen_parameters()
        {
            std::bernoulli_distribution dist(0.6);
            gen_parameter();
            if(dist(engine))
            {
                push(tt::comma);
                gen_parameters();
            }
        }

        void gen_parameter()
        {
            gen_type();
            push_id();
        }

        void gen_type()
        {
            std::bernoulli_distribution dist(0.5);
            if(dist(engine))
            {
                gen_type();
                push(tt::left_bracket);
                push(tt::right_bracket);
            }else{
                gen_basic_type();
            }
        }

        void gen_basic_type()
        {
            std::uniform_int_distribution<int> dist(0,4);
            switch(dist(engine))
            {
            case 0: return push(tt::kw_int);
            case 1: return push(tt::kw_boolean);
            case 2: return push(tt::kw_void);
            default:
                push_Id();
            }
        }

        void gen_statement()
        {
            std::uniform_int_distribution<int> dist(0, 5);
            switch(dist(engine))
            {
            case 0: return gen_block();
            case 1: return gen_empty_statement();
            case 2: return gen_if_statement();
            case 3: return gen_expression_statement();
            case 4: return gen_while_statement();
            case 5: return gen_return_statement();
            }
        }

        void gen_block()
        {
            auto dist = std::exponential_distribution<double>{0.7};
            auto n_blocks = static_cast<int>(dist(engine));
            push(tt::left_brace);
            while(n_blocks--)
            {
                gen_block_statement();
            }
            push(tt::right_brace);
        }

        void gen_block_statement()
        {
            std::bernoulli_distribution dist(0.9);

            if(dist(engine))
            {
                gen_statement();
            }else{
                gen_local_variable_statement();
            }
        }

        void gen_local_variable_statement()
        {
            std::bernoulli_distribution dist(0.7);

            gen_type();
            push_id();
            if(dist(engine))
            {
                push(tt::assign);
                gen_expression();
            }
            push(tt::semicolon);
        }

        void gen_empty_statement()
        {
            push(tt::semicolon);
        }

        void gen_while_statement()
        {
            push(tt::kw_while);
            push(tt::left_paren);
            gen_expression();
            push(tt::right_paren);
            gen_statement();
        }

        void gen_if_statement()
        {
            std::bernoulli_distribution dist(0.5);

            push(tt::kw_if);
            push(tt::left_paren);
            gen_expression();
            push(tt::right_paren);
            gen_statement();
            if(dist(engine))
            {
                push(tt::kw_else);
                gen_statement();
            }
        }

        void gen_expression_statement()
        {
            gen_expression();
            push(tt::semicolon);
        }

        void gen_return_statement()
        {
            std::bernoulli_distribution dist(0.8);
            push(tt::kw_return);
            if(dist(engine))
            {
                gen_expression();
            }
            push(tt::semicolon);
        }

        void gen_expression()
        {
            gen_assignment_expression();
        }

        void gen_assignment_expression()
        {
            std::bernoulli_distribution dist(get_expr_dist_param());
            gen_logical_or_expression();
            if(dist(engine))
            {
                push(tt::assign);
                gen_assignment_expression();
            }
        }

        void gen_logical_or_expression()
        {
            std::bernoulli_distribution dist(get_expr_dist_param());
            if(dist(engine))
            {
                gen_logical_or_expression();
                push(tt::logical_or);
            }
            gen_logical_and_expression();
        }

        void gen_logical_and_expression()
        {
            std::bernoulli_distribution dist(get_expr_dist_param());
            if(dist(engine))
            {
                gen_logical_and_expression();
                push(tt::logical_and);
            }
            gen_equality_expression();
        }

        void gen_equality_expression()
        {
            std::bernoulli_distribution dist(get_expr_dist_param());
            if(dist(engine))
            {
                gen_equality_expression();
                push_one(tt::equal, tt::not_equal);
            }
            gen_relational_expression();
        }

        void gen_relational_expression()
        {
            std::bernoulli_distribution dist(get_expr_dist_param());
            if(dist(engine))
            {
                gen_relational_expression();
                push_one(tt::less_than, tt::less_equal, tt::greater_than, tt::greater_equal);
            }
            gen_additive_expression();
        }

        void gen_additive_expression()
        {
            std::bernoulli_distribution dist(get_expr_dist_param());
            if(dist(engine))
            {
                gen_additive_expression();
                push_one(tt::plus, tt::minus);
            }
            gen_multiplicative_expression();
        }

        void gen_multiplicative_expression()
        {
            std::bernoulli_distribution dist(get_expr_dist_param());
            if(dist(engine))
            {
                gen_multiplicative_expression();
                push_one(tt::multiply, tt::divides, tt::modulo);
            }
            gen_unary_expression();
        }

        void gen_unary_expression()
        {
            std::bernoulli_distribution dist(1 - get_expr_dist_param());
            if(dist(engine))
            {
                gen_postfix_expression();
            }else{

                push_one(tt::logical_not, tt::minus);
                gen_unary_expression();
            }
        }

        void gen_postfix_expression()
        {
            std::exponential_distribution<double> dist(1.0);
            auto n_postfixops = static_cast<int>(dist(engine));

            gen_primary_expression();
            while(n_postfixops--)
            {
                gen_postfix_op();
            }
        }

        void gen_postfix_op()
        {
            std::uniform_int_distribution<int> dist(0, 2);
            switch(dist(engine))
            {
            case 0: return gen_method_invokation();
            case 1: return gen_field_access();
            case 2: return gen_array_access();
            }
        }

        void gen_method_invokation()
        {
            push(tt::dot);
            push_id();
            push(tt::left_paren);
            gen_arguments();
            push(tt::right_paren);
        }

        void gen_field_access()
        {
            push(tt::dot);
            push_id();
        }

        void gen_array_access()
        {
            push(tt::left_bracket);
            gen_expression();
            push(tt::right_bracket);
        }

        void gen_arguments()
        {
            std::bernoulli_distribution empty_dist(0.2);
            if(!empty_dist(engine))
            {
                std::exponential_distribution<> dist(1);
                auto n_args = static_cast<int>(dist(engine));
                gen_expression();
                while(n_args--)
                {
                    push(tt::comma);
                    gen_expression();
                }
            }
        }

        void gen_primary_expression()
        {
            std::uniform_int_distribution<> dist(0,9);
            switch(dist(engine))
            {
            case 0: return push(tt::kw_null);
            case 1: return push(tt::kw_false);
            case 2: return push(tt::kw_true);
            case 3: return push_lit();
            case 4: return push_id();
            case 5:
                push_id();
                push(tt::left_paren);
                gen_arguments();
                push(tt::right_paren);
                return;
            case 6: return push(tt::kw_this);
            case 7:
                push(tt::left_paren);
                gen_expression();
                push(tt::right_paren);
                return;
            case 8: return gen_new_obj_expression();
            case 9: return gen_new_array_expression();
            }
        }

        void gen_new_obj_expression()
        {
            push(tt::kw_new);
            push_Id();
            push(tt::left_paren);
            push(tt::right_paren);
        }

        void gen_new_array_expression()
        {
            std::exponential_distribution<> dist{1};
            auto rank = static_cast<int>(dist(engine));

            push(tt::kw_new);
            gen_basic_type();
            push(tt::left_bracket);
            gen_expression();
            push(tt::right_bracket);

            while(rank--)
            {
                push(tt::left_bracket);
                push(tt::right_bracket);
            }
        }

        template<typename... T>
        void push_one(T... args)
        {
            static_assert(sizeof...(args) > 0, "");
            tt tts[] = { args... };
            std::uniform_int_distribution<std::size_t> dist(0, sizeof...(args) - 1);
            push(tts[dist(engine)]);
        }

        void push_id()
        {
            push_id("foo");
        }

        void push_Id()
        {
            push_id("Foo");
        }

        void push_id(const std::string& id)
        {
            tokens.push_back(minijava::token::create_identifier(g_pool.normalize(id)));
            output(tokens.back());
        }

        void push_lit()
        {
            std::uniform_int_distribution<> dist(0, 100);
            tokens.push_back(minijava::token::create_integer_literal(g_pool.normalize(std::to_string(dist(engine)))));
            output(tokens.back());
        }

        void push(const tt type)
        {
            tokens.push_back(minijava::token::create(type));
            output(tokens.back());
        }

        void output(const minijava::token& tok)
        {
            if(tok.type() == tt::right_brace)
            {
                --indent;
                nextline();
            }

            std::cout << stringify(tok) << " ";

            if(tok.type() == tt::left_brace)
            {
                ++indent;
                nextline();
            }else if (tok.type() == tt::semicolon)
            {
                nextline();
            }else if (tok.type() == tt::right_brace)
            {
                nextline();
            }
        }


        void nextline()
        {
            std::cout << "\n";
            for(int i = 0; i < indent; ++i)
                std::cout << indent_str;
        }

        std::string stringify(const minijava::token t)
        {
            return t.has_lexval()
                ? t.lexval().c_str()
                : name(t.type());
        }

        std::vector<minijava::token> tokens{};
        std::default_random_engine engine{};

        std::string indent_str = "  ";
        int indent = 0;
    };

    std::vector<minijava::token> generate_valid_program()
    {
        generator gen{};
        std::cout << "/* seed: " << gen.engine << "*/" << std::endl;
        gen.gen_program();
        return std::move(gen.tokens);
    }
}


int main()
{
    const auto tokens = generate_valid_program();

    minijava::parse_program(std::begin(tokens), std::end(tokens));
}
