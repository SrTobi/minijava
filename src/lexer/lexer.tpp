#ifndef MINIJAVA_INCLUDED_FROM_LEXER_LEXER_HPP
#error "Never `#include <lexer/lexer.tpp>` directly; `#include <lexer.hpp>` instead."
#endif

#include <cctype>

#include "lexer/keyword.hpp"


namespace minijava
{

	namespace detail
	{

		inline bool isspace(int c) noexcept
		{
			return (c == ' ') || (c == '\r') || (c == '\n') || (c == '\t');
		}

		inline bool isdigit(const int c) noexcept
		{
			return std::isdigit(c);
		}

		inline bool isidhead(const int c) noexcept
		{
			return ((c == '_') || std::isalpha(c));
		}

		inline bool isidtail(const int c) noexcept
		{
			return ((c == '_') || std::isalnum(c));
		}

	}

	template<typename InIterT, typename SymPoolT>
	lexer<InIterT, SymPoolT>::lexer(const InIterT first,
									const InIterT last,
									SymPoolT& id_pool,
									SymPoolT& lit_pool) :
		_current_token{token::create(token_type::eof)},
		_current_it{first}, _last_it{last}, _id_pool{id_pool}, _lit_pool{lit_pool}
	{
		advance();
	}

	template<typename InIterT, typename SymPoolT>
	const token& lexer<InIterT, SymPoolT>::current_token() const noexcept
	{
		return _current_token;
	}

	template<typename InIterT, typename SymPoolT>
	bool lexer<InIterT, SymPoolT>::current_token_is_eof() const noexcept
	{
		return (current_token().type() == token_type::eof);
	}

	template<typename InIterT, typename SymPoolT>
	void lexer<InIterT, SymPoolT>::advance()
	{
		while (!_maybe_advance()) {
			// Try again, eh?
		}
	}

	template<typename InIterT, typename SymPoolT>
	bool lexer<InIterT, SymPoolT>::_maybe_advance()
	{
		const auto c = _skip_white_space();
		if (c < 0) {
			_current_token = token::create(token_type::eof);
		} else if (detail::isidhead(c)) {
			_scan_identifier();
		} else if (detail::isdigit(c)) {
			_scan_integer_literal();
		} else if (c == '/') {
			const auto next = _next();
			if (next == '*') {
				// Skip '*' from opening '/*' to not confuse it as part of closing '*/'.
				_skip();
				_skip_block_comment();
				return false;
			} else if (next == '=') {
				_current_token = token::create(token_type::divides_assign);
				_skip();
			} else {
				_current_token = token::create(token_type::divides);
			}
		} else if (_maybe_token('[', token_type::left_bracket)) {
		} else if (_maybe_token(']', token_type::right_bracket)) {
		} else if (_maybe_token('(', token_type::left_paren)) {
		} else if (_maybe_token(')', token_type::right_paren)) {
		} else if (_maybe_token('{', token_type::left_brace)) {
		} else if (_maybe_token('}', token_type::right_brace)) {
		} else if (_maybe_token('.', token_type::dot)) {
		} else if (_maybe_token(',', token_type::comma)) {
		} else if (_maybe_token(':', token_type::colon)) {
		} else if (_maybe_token(';', token_type::semicolon)) {
		} else if (_maybe_token('?', token_type::conditional)) {
		} else if (_maybe_token('~', token_type::bit_not)) {
		} else if (_maybe_token('!', token_type::logical_not)) {
			_maybe_token('=', token_type::not_equal);
		} else if (_maybe_token('*', token_type::multiply)) {
			_maybe_token('=', token_type::multiply_assign);
		} else if (_maybe_token('+', token_type::plus)) {
			if (!_maybe_token('+', token_type::increment)) {
				_maybe_token('=', token_type::plus_assign);
			}
		} else if (_maybe_token('-', token_type::minus)) {
			if (!_maybe_token('-', token_type::decrement)) {
				_maybe_token('=', token_type::minus_assign);
			}
		} else if (_maybe_token('<', token_type::less_than)) {
			if (_maybe_token('<', token_type::left_shift)) {
				_maybe_token('=', token_type::left_shift_assign);
			} else {
				_maybe_token('=', token_type::less_equal);
			}
		} else if (_maybe_token('=', token_type::assign)) {
			_maybe_token('=', token_type::equal);
		} else if (_maybe_token('>', token_type::greater_than)) {
			if (_maybe_token('>', token_type::right_shift)) {
				if (_maybe_token('>', token_type::unsigned_right_shift)) {
					_maybe_token('=', token_type::unsigned_right_shift_assign);
				} else {
					_maybe_token('=', token_type::right_shift_assign);
				}
			} else {
				_maybe_token('=', token_type::greater_equal);
			}
		} else if (_maybe_token('%', token_type::modulo)) {
			_maybe_token('=', token_type::modulo_assign);
		} else if (_maybe_token('&', token_type::bit_and)) {
			if (!_maybe_token('&', token_type::logical_and)) {
				_maybe_token('=', token_type::bit_and_assign);
			}
		} else if (_maybe_token('^', token_type::bit_xor)) {
			_maybe_token('=', token_type::bit_xor_assign);
		} else if (_maybe_token('|', token_type::bit_or)) {
			if (!_maybe_token('|', token_type::logical_or)) {
				_maybe_token('=', token_type::bit_or_assign);
			}
		} else {
			throw lexical_error{};
		}
		return true;
	}

	template<typename InIterT, typename SymPoolT>
	void lexer<InIterT, SymPoolT>::_scan_identifier()
	{
		auto buffer = std::string{};
		auto c = _current();
		assert(detail::isidhead(c));
		do {
			buffer.push_back(static_cast<char>(c));
			c = _next();
		} while (detail::isidtail(c));
		const auto tt = classify_word(buffer);
		if (tt == token_type::identifier) {
			const auto lexval = _id_pool.normalize(buffer);
			_current_token = token::create_identifier(lexval);
		} else {
			assert(category(tt) == token_category::keyword);
			_current_token = token::create(tt);
		}
	}

	template<typename InIterT, typename SymPoolT>
	void lexer<InIterT, SymPoolT>::_scan_integer_literal()
	{
		auto buffer = std::string{};
		auto c = _current();
		assert(detail::isdigit(c));
		do {
			buffer.push_back(static_cast<char>(c));
			c = _next();
		} while (detail::isdigit(c));
		if ((buffer.front() == '0') && (buffer.size() > 1)) {
			throw lexical_error{"Invalid integer literal: leading zeros are not allowed"};
		}
		const auto lexval = _lit_pool.normalize(buffer);
		_current_token = token::create_integer_literal(lexval);
	}

	template<typename InIterT, typename SymPoolT>
	int lexer<InIterT, SymPoolT>::_skip_block_comment()
	{
		enum dfa_state {q0, q1, q2};
		auto state = q0;
		for (auto c = _current(); true; c = _next()) {
			switch (state) {
			case q0:
				switch (c) {
				case '*': state = q1; continue;
				}
				break;
			case q1:
				switch (c) {
				case '*':             continue;
				case '/': state = q2; continue;
				default:  state = q0;
				}
				break;
			case q2:
				return c;
			}
			if (c < 0) {
				throw lexical_error{"Input ended before block-comment was closed"};
			}
		}
	}

	template<typename InIterT, typename SymPoolT>
	int lexer<InIterT, SymPoolT>::_skip_white_space() noexcept
	{
		auto c = _current();
		while (detail::isspace(c)) {
			c = _next();
		}
		return c;
	}

	template<typename InIterT, typename SymPoolT>
	lexer<InIterT, SymPoolT> make_lexer(const InIterT first,
										const InIterT last,
										SymPoolT& id_pool,
										SymPoolT& lit_pool)
	{
		return lexer<InIterT, SymPoolT>{first, last, id_pool, lit_pool};
	}

}  // namespace minijava
