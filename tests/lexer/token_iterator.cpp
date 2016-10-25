#include "lexer/token_iterator.hpp"

#include <algorithm>
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
