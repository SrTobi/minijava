#include "lexer/token_iterator.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#define BOOST_TEST_MODULE  lexer_token_iterator
#include <boost/test/unit_test.hpp>

namespace /* anonymous */
{

	class stub_lexer
	{
	public:
		stub_lexer(int last) : _last{last} {}
		void advance() { _current += 1; }
		const int& current_token() const { return _current; }
		bool current_token_is_eof() const { return _current == _last; }
	private:
		int _current{};
		int _last{};
	};

	struct stub_lexer_error : public std::runtime_error
	{
		stub_lexer_error() : std::runtime_error{"invalid input"} {}
	};

	class throwing_stub_lexer
	{
	public:
		throwing_stub_lexer(int error_position) : _error_position{error_position} {}
		void advance() { _current += 1; if (_current == _error_position) throw stub_lexer_error(); }
		const int& current_token() const { return _current; }
		bool current_token_is_eof() const { return false; }
	private:
		int _current{};
		int _error_position{};
	};

}


BOOST_AUTO_TEST_CASE(iterates_correctly)
{
	auto lexer = stub_lexer{5};
	const int expected[] = {0, 1, 2, 3, 4, 5};
	BOOST_REQUIRE(std::equal(std::begin(expected),
							 std::end(expected),
							 minijava::token_begin(lexer),
							 minijava::token_end(lexer)));
}


BOOST_AUTO_TEST_CASE(iterates_empty_range_correctly)
{
	auto lexer = stub_lexer{0};
	const int expected[] = {0};
	BOOST_REQUIRE(std::equal(std::begin(expected),
							 std::end(expected),
							 minijava::token_begin(lexer),
							 minijava::token_end(lexer)));
}


BOOST_AUTO_TEST_CASE(iterates_up_to_error)
{
	auto lexer = throwing_stub_lexer{5};
	auto it = minijava::token_begin(lexer);
	for (int i = 0; i < 4; ++i, ++it) {
		BOOST_REQUIRE_EQUAL(*it, i);
	}
	BOOST_REQUIRE_EQUAL(*it, 4);
	BOOST_REQUIRE_THROW(++it, stub_lexer_error);
}
