#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>

#include "exceptions.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "parser/serializer.hpp"
#include "symbol_pool.hpp"


namespace po = boost::program_options;

using tt = minijava::token_type;


namespace /* anonymous */
{

	minijava::symbol_pool<> g_pool{};

	template <typename T>
	struct restore_finally
	{

		restore_finally() = default;

		restore_finally(T& dest, const T update) : _destp{&dest}, _prev{dest}
		{
			dest = update;
		}

		~restore_finally()
		{
			if (_destp != nullptr) {
				*_destp = _prev;
			}
		}

		restore_finally(const restore_finally&) = delete;

		restore_finally(restore_finally&& other) noexcept : restore_finally{}
		{
			swap(*this, other);
		}

		restore_finally& operator=(const restore_finally&) = delete;

		restore_finally& operator=(restore_finally&& other) noexcept
		{
			restore_finally temp{};
			swap(*this, temp);
			swap(*this, other);
			return *this;
		}

		friend void swap(restore_finally& lhs, restore_finally& rhs) noexcept
		{
			std::swap(lhs._destp, rhs._destp);
			std::swap(lhs._prev, rhs._prev);
		}

	private:

		T * _destp{};
		T _prev{};

	};


	struct generator
	{

		generator(const unsigned seed, const std::size_t limit)
			: engine{seed}
			, nest_depth{0}
			, nest_limit{limit}
		{
		}

		void gen_program()
		{
			const auto guard = enter_nested();
			while (nest_deeper_eh()) {
				gen_class_decl();
			}
			tokens.push_back(minijava::token::create(tt::eof));
		}

		void gen_class_decl()
		{
			const auto guard = enter_nested();
			push(tt::kw_class);
			push_Id();
			push(tt::left_brace);
			while (nest_deeper_eh()) {
				gen_class_member();
			}
			push(tt::right_brace);
		}

		void gen_class_member()
		{
			const auto guard = enter_nested();
			std::uniform_int_distribution<int> dist{0, 5};
			switch(dist(engine)) {
			case 0:
			case 1:
				gen_field();
				return;
			case 2:
			case 3:
				gen_method();
				return;
			default:
				gen_main_method();
				return;
			}
		}

		void gen_field()
		{
			const auto guard = enter_nested();
			push(tt::kw_public);
			gen_type();
			push_id();
			push(tt::semicolon);
		}

		void gen_main_method()
		{
			const auto guard = enter_nested();
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
			const auto guard = enter_nested();
			push(tt::kw_public);
			gen_type();
			push_id();
			push(tt::left_paren);
			if (nest_deeper_eh()) {
				gen_parameters();
			}
			push(tt::right_paren);
			gen_block();
		}

		void gen_parameters()
		{
			const auto guard = enter_nested();
			gen_parameter();
			if (nest_deeper_eh()) {
				push(tt::comma);
				gen_parameters();
			}
		}

		void gen_parameter()
		{
			const auto guard = enter_nested();
			gen_type();
			push_id();
		}

		void gen_type()
		{
			const auto guard = enter_nested();
			if (nest_deeper_eh()) {
				gen_type();
				push(tt::left_bracket);
				push(tt::right_bracket);
			} else {
				gen_basic_type();
			}
		}

		void gen_basic_type()
		{
			const auto guard = enter_nested();
			auto dist = std::uniform_int_distribution<int>{0, 4};
			switch(dist(engine)) {
			case 0:
				push(tt::kw_int);
				return;
			case 1:
				push(tt::kw_boolean);
				return;
			case 2:
				push(tt::kw_void);
				return;
			default:
				push_Id();
				return;
			}
		}

		void gen_statement()
		{
			const auto guard = enter_nested();
			auto dist = std::uniform_int_distribution<int>{0, 5};
			switch(dist(engine)) {
			case 0:
				gen_block();
				return;
			case 1:
				gen_empty_statement();
				return;
			case 2:
				gen_if_statement();
				return;
			case 3:
				gen_expression_statement();
				return;
			case 4:
				gen_while_statement();
				return;
			case 5:
				gen_return_statement();
				return;
			}
		}

		void gen_block()
		{
			const auto guard = enter_nested();
			push(tt::left_brace);
			while (nest_deeper_eh()) {
				gen_block_statement();
			}
			push(tt::right_brace);
		}

		void gen_block_statement()
		{
			const auto guard = enter_nested();
			auto dist = std::bernoulli_distribution{0.7};
			if (dist(engine)) {
				gen_statement();
			} else {
				gen_local_variable_statement();
			}
		}

		void gen_local_variable_statement()
		{
			const auto guard = enter_nested();
			gen_type();
			push_id();
			if (nest_deeper_eh()) {
				push(tt::assign);
				gen_expression();
			}
			push(tt::semicolon);
		}

		void gen_empty_statement()
		{
			const auto guard = enter_nested();
			push(tt::semicolon);
		}

		void gen_while_statement()
		{
			const auto guard = enter_nested();
			push(tt::kw_while);
			push(tt::left_paren);
			gen_expression();
			push(tt::right_paren);
			gen_statement();
		}

		void gen_if_statement()
		{
			const auto guard = enter_nested();
			push(tt::kw_if);
			push(tt::left_paren);
			gen_expression();
			push(tt::right_paren);
			gen_statement();
			if (nest_deeper_eh()) {
				push(tt::kw_else);
				gen_statement();
			}
		}

		void gen_expression_statement()
		{
			const auto guard = enter_nested();
			gen_expression();
			push(tt::semicolon);
		}

		void gen_return_statement()
		{
			const auto guard = enter_nested();
			push(tt::kw_return);
			if (nest_deeper_eh()) {
				gen_expression();
			}
			push(tt::semicolon);
		}

		void gen_expression()
		{
			const auto guard = enter_nested();
			gen_assignment_expression();
		}

		void gen_assignment_expression()
		{
			const auto guard = enter_nested();
			gen_logical_or_expression();
			if (nest_deeper_eh()) {
				push(tt::assign);
				gen_assignment_expression();
			}
		}

		void gen_logical_or_expression()
		{
			const auto guard = enter_nested();
			if (nest_deeper_eh()) {
				gen_logical_or_expression();
				push(tt::logical_or);
			}
			gen_logical_and_expression();
		}

		void gen_logical_and_expression()
		{
			const auto guard = enter_nested();
			if (nest_deeper_eh()) {
				gen_logical_and_expression();
				push(tt::logical_and);
			}
			gen_equality_expression();
		}

		void gen_equality_expression()
		{
			const auto guard = enter_nested();
			if (nest_deeper_eh()) {
				gen_equality_expression();
				push_one(tt::equal, tt::not_equal);
			}
			gen_relational_expression();
		}

		void gen_relational_expression()
		{
			const auto guard = enter_nested();
			if (nest_deeper_eh()) {
				gen_relational_expression();
				push_one(tt::less_than, tt::less_equal, tt::greater_than, tt::greater_equal);
			}
			gen_additive_expression();
		}

		void gen_additive_expression()
		{
			const auto guard = enter_nested();
			if (nest_deeper_eh()) {
				gen_additive_expression();
				push_one(tt::plus, tt::minus);
			}
			gen_multiplicative_expression();
		}

		void gen_multiplicative_expression()
		{
			const auto guard = enter_nested();
			if (nest_deeper_eh()) {
				gen_multiplicative_expression();
				push_one(tt::multiply, tt::divides, tt::modulo);
			}
			gen_unary_expression();
		}

		void gen_unary_expression()
		{
			const auto guard = enter_nested();
			if (nest_deeper_eh()) {
				push_one(tt::logical_not, tt::minus);
				gen_unary_expression();
			} else {
				gen_postfix_expression();
			}
		}

		void gen_postfix_expression()
		{
			const auto guard = enter_nested();
			gen_primary_expression();
			while (nest_deeper_eh()) {
				gen_postfix_op();
			}
		}

		void gen_postfix_op()
		{
			const auto guard = enter_nested();
			auto dist = std::uniform_int_distribution<int>{0, 2};
			switch (dist(engine)) {
			case 0:
				gen_method_invokation();
				return;
			case 1:
				gen_field_access();
				return;
			case 2:
				gen_array_access();
				return;
			default:
				MINIJAVA_NOT_REACHED();
			}
		}

		void gen_method_invokation()
		{
			const auto guard = enter_nested();
			push(tt::dot);
			push_id();
			push(tt::left_paren);
			gen_arguments();
			push(tt::right_paren);
		}

		void gen_field_access()
		{
			const auto guard = enter_nested();
			push(tt::dot);
			push_id();
		}

		void gen_array_access()
		{
			const auto guard = enter_nested();
			push(tt::left_bracket);
			gen_expression();
			push(tt::right_bracket);
		}

		void gen_arguments()
		{
			const auto guard = enter_nested();
			if (nest_deeper_eh()) {
				gen_expression();
				while (nest_deeper_eh()) {
					push(tt::comma);
					gen_expression();
				}
			}
		}

		void gen_primary_expression()
		{
			const auto guard = enter_nested();
			auto dist = std::uniform_int_distribution<int>{0, 9};
			switch (dist(engine)) {
			case 0:
				push(tt::kw_null);
				return;
			case 1:
				push(tt::kw_false);
				return;
			case 2:
				push(tt::kw_true);
				return;
			case 3:
				push_lit();
				return;
			case 4:
				push_id();
				return;
			case 5:
				push_id();
				push(tt::left_paren);
				gen_arguments();
				push(tt::right_paren);
				return;
			case 6:
				push(tt::kw_this);
				return;
			case 7:
				push(tt::left_paren);
				gen_expression();
				push(tt::right_paren);
				return;
			case 8:
				gen_new_obj_expression();
				return;
			case 9:
				gen_new_array_expression();
				return;
			}
		}

		void gen_new_obj_expression()
		{
			const auto guard = enter_nested();
			push(tt::kw_new);
			push_Id();
			push(tt::left_paren);
			push(tt::right_paren);
		}

		void gen_new_array_expression()
		{
			const auto guard = enter_nested();
			auto dist = std::exponential_distribution<>{1};
			auto rank = static_cast<int>(dist(engine));
			push(tt::kw_new);
			gen_basic_type();
			push(tt::left_bracket);
			gen_expression();
			push(tt::right_bracket);
			while (rank--) {
				push(tt::left_bracket);
				push(tt::right_bracket);
			}
		}

		template<typename... T>
		void push_one(T... args)
		{
			static_assert(sizeof...(args) > 0, "");
			const tt tts[] = { args... };
			auto dist = std::uniform_int_distribution<std::size_t>{0, sizeof...(args) - 1};
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
		}

		void push_lit()
		{
			auto dist = std::uniform_int_distribution<int>{0, 100};
			const auto lexval = g_pool.normalize(std::to_string(dist(engine)));
			tokens.push_back(minijava::token::create_integer_literal(lexval));
		}

		void push(const tt type)
		{
			tokens.push_back(minijava::token::create(type));
		}

		bool nest_deeper_eh()
		{
			if (nest_depth >= nest_limit) {
				return false;
			}
			const auto p = 1.0 - std::sqrt(
				static_cast<double>(nest_depth) / static_cast<double>(nest_limit)
			);
			auto dist = std::bernoulli_distribution{p};
			return dist(engine);
		}

		restore_finally<std::size_t> enter_nested()
		{
			return {nest_depth, nest_depth + 1};
		}

		std::vector<minijava::token> tokens{};

		std::default_random_engine engine{};

		std::size_t nest_depth{};

		std::size_t nest_limit{};

	};  // struct generator


	std::vector<minijava::token> generate_valid_program(const unsigned seed, const std::size_t depth)
	{
		auto gen = generator{seed, depth};
		gen.gen_program();
		return std::move(gen.tokens);
	}

	unsigned get_random_seed()
	{
		std::random_device randdev{};
		return randdev();
	}

}


int main(int argc, char * * argv)
{
	try {
		auto noprint = false;
		auto seed = 0U;
		auto limit = std::size_t{20};
		auto options = po::options_description{"Options"};
		options.add_options()
			(
				"help",
				"show help text and exit"
			)(
				"no-print",
				po::bool_switch(&noprint),
				"don't serialize and write the program to standard output"
			)(
				"random-seed",
				po::value<unsigned>(&seed),
				"use explicit random seed instead of non-determinism"
			)(
				"recursion-limit",
				po::value<std::size_t>(&limit),
				"set limit for recursion depth"
			);
		auto varmap = po::variables_map{};
		po::store(po::parse_command_line(argc, argv, options), varmap);
		po::notify(varmap);
		if (varmap.count("help")) {
			std::cout
				<< "usage: generator [--random-seed=VALUE] [--recursion-limit=N]\n"
				<< "\n"
				<< "Generates a syntactically correct random MiniJava program.\n"
				<< "\n"
				<< options << "\n"
				<< "\n"
				<< "The program is validated internally with the MiniJava parser\n"
				<< "after it is generated and then serialized to standard output.\n";
			return EXIT_SUCCESS;
		}
		if (varmap.count("random-seed") == 0) {
			seed = get_random_seed();
		}
		const auto tokens = generate_valid_program(seed, limit);
		std::cout << "/* random seed was:      " << seed << " */\n"
				  << "/* recursion limit was:  " << limit << " */\n\n";
		if (!noprint) {
			minijava::pretty_print(std::cout, std::begin(tokens), std::end(tokens));
			std::cout << '\n';
		}
		try {
			minijava::parse_program(std::begin(tokens), std::end(tokens));
		} catch (const minijava::syntax_error& e) {
			std::cerr << "error: " << e.what() << "\n";
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	} catch (const std::exception& e) {
		std::cerr << "error: " << e.what() << "\n";
		return EXIT_FAILURE;
	}
}
