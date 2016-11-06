#include "parser/parser.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>

#include <boost/algorithm/string/join.hpp>


namespace minijava
{

	syntax_error::syntax_error()
		: std::runtime_error{"invalid syntax"}
	{
	}

	syntax_error::syntax_error(const std::string msg)
		: std::runtime_error{msg}
	{
	}

	syntax_error::syntax_error(const token& tok, const std::string& msg)
		: std::runtime_error{msg}
		, _line{tok.line()}
		, _column{tok.column()}
	{
	}


	namespace /* anonymous */
	{

		void insert_pretty(std::ostream& os, const token& tok)
		{
			switch (const auto cat = category(tok.type())) {
			case token_category::identifier:
			case token_category::literal:
				os << name(cat) << " " << "'" << tok.lexval() << "'";
				break;
			case token_category::keyword:
				os << name(cat) << " " << "'" << name(tok.type()) << "'";
				break;
			case token_category::punctuation:
				os << "'" << name(tok.type()) << "'";
				break;
			default:
				os << name(tok.type());
				break;
			}
		}

		void insert_pretty(std::ostream& os, const token_type tt)
		{
			switch (const auto cat = category(tt)) {
			case token_category::keyword:
				os << name(cat) << " '" << name(tt) << "'";
				break;
			case token_category::punctuation:
				os << "'" << name(tt) << "'";
				break;
			default:
				os << name(tt);
				break;
			}
		}

	}  // namespace /* anonymous */


	namespace detail
	{

		[[noreturn]]
		void throw_syntax_error(const token& pde,
		                        const std::initializer_list<token_type> expected)
		{
			const auto first = std::begin(expected);
			const auto last = std::end(expected);
			const auto expected_count = std::distance(first, last);
			assert(expected_count > 0);
			assert(std::find(first, last, pde.type()) == last);
			auto oss = std::ostringstream{};
			oss << "Expected ";
			insert_pretty(oss, *first);
			if (expected_count > 2) {
				const auto append = [&oss](auto&& tt){
					oss << ", ";
					insert_pretty(oss, tt);
				};
				std::for_each(std::next(first), std::prev(last), append);
			}
			if (expected_count > 1) {
				oss << " or ";
				insert_pretty(oss, *std::prev(last));
			}
			oss << " but found ";
			insert_pretty(oss, pde);
			throw syntax_error{pde, oss.str()};
		}

		[[noreturn]]
		void throw_syntax_error_main_signature(const token& pde)
		{
			assert(pde.type() == token_type::identifier);
			throw syntax_error{
				pde,
				"The 'main' method must be declared as"
				" 'public static void main(String[] args)'"
			};
		}

		[[noreturn]]
		void throw_syntax_error_new_primitive(const token& pde, const token_type tt)
		{
			using namespace std::string_literals;
			assert(pde.type() == token_type::left_paren);
			const auto msg = "Primitive type"s
				+ " '" + name(tt) + "' "
				+ "cannot be used in new object expression";
			throw syntax_error{pde, msg};
		}

	}  // namespace detail

}  // namespace minijava
