#ifndef MINIJAVA_INCLUDED_FROM_LEXER_LEXER_HPP
#error "Never `#include <lexer/lexer.tpp>` directly; `#include <lexer.hpp>` instead."
#endif

// TODO @Philipp Serrer: Implement a working lexer.

#include <cctype>


namespace minijava
{

	template<typename InIterT, typename StrPoolT>
	lexer<InIterT, StrPoolT>::lexer(const InIterT first,
									const InIterT last,
									StrPoolT& pool) :
		_current_token{token::create(token_type::eof)},
		_next{first}, _last{last}, _id_pool{pool}
	{
		advance();
	}

	template<typename InIterT, typename StrPoolT>
	const token& lexer<InIterT, StrPoolT>::current_token() const noexcept
	{
		return _current_token;
	}

	template<typename InIterT, typename StrPoolT>
	void lexer<InIterT, StrPoolT>::advance()
	{
		// This implementation is fo course complete crap.  But it is good
		// enough to pass a simple unit-test.
		while ((_next != _last) && std::isspace(*_next)) {
			_next++;
		}
		if (_next == _last) {
			_current_token = token::create(token_type::eof);
			return;
		}
		auto buffer = std::string{};
		while ((_next != _last) && !std::isspace(*_next)) {
			if (!std::isalpha(*_next)) {
				throw lexical_error{};
			}
			buffer += *_next;
			_next++;
		}
		const auto canon = _id_pool.normalize(buffer);
		_current_token = token::create_identifier(canon);
	}

	template<typename InIterT, typename StrPoolT>
	bool lexer<InIterT, StrPoolT>::current_token_is_eof() const noexcept
	{
		return (current_token().type() == token_type::eof);
	}

	template<typename InIterT, typename StrPoolT>
	lexer<InIterT, StrPoolT> make_lexer(const InIterT first,
										const InIterT last,
										StrPoolT& pool)
	{
		return lexer<InIterT, StrPoolT>{first, last, pool};
	}

}  // namespace minijava
