#ifndef MINIJAVA_INCLUDED_FROM_LEXER_LEXER_HPP
#error "Never `#include <lexer/lexer.tpp>` directly; `#include <lexer.hpp>` instead."
#endif

#include <cctype>

namespace minijava
{

	template<typename InIterT, typename StrPoolT>
	lexer<InIterT, StrPoolT>::lexer(const InIterT first,
									const InIterT last,
									StrPoolT& pool) :
		_current_token{token::create(token_type::eof)},
		_current_it{first}, _last_it{last}, _id_pool{pool}
	{
		advance();
		_line = 1;
		_column = 1;
	}

	template<typename InIterT, typename StrPoolT>
	const token& lexer<InIterT, StrPoolT>::current_token() const noexcept
	{
		return _current_token;
	}

	template<typename InIterT, typename StrPoolT>
	void lexer<InIterT, StrPoolT>::advance()
	{
		if (_current_is_last()) {
			_current_token = token::create(token_type::eof);
			return;
		}


		// skip whitespaces
		while (_isspace(_current())) _skip();

		// store token start position
		auto line = _line;
		auto column = _column;

		auto c = _current();
		if (std::isalpha(c)) {
			// scan identifier
			_scan_identifier();
		} else if (std::isdigit(c)) {
			// scan digit
			_scan_integer();
		} else if (c == '/') {
			auto next_char = _next();
			// comment?
			if (next_char == '*') {
				// skip comment and run advance again
				_consume_block_comment();
				advance();
				return;
			} else if (next_char == '=') {
				// /= token
				_current_token = token::create(token_type::divides_assign);
				_skip();
			} else {
				// / token
				_current_token = token::create(token_type::divides);
			}
		}
		else if (_maybe_token('[', token_type::left_bracket)) {}
		else if (_maybe_token(']', token_type::right_bracket)) {}
		else if (_maybe_token('(', token_type::left_paren)) {}
		else if (_maybe_token(')', token_type::right_paren)) {}
		else if (_maybe_token('{', token_type::left_brace)) {}
		else if (_maybe_token('}', token_type::right_brace)) {}
		else if (_maybe_token('.', token_type::dot)) {}
		else if (_maybe_token(',', token_type::comma)) {}
		else if (_maybe_token(':', token_type::colon)) {}
		else if (_maybe_token(';', token_type::semicolon)) {}
		else if (_maybe_token('?', token_type::conditional)) {}
		else if (_maybe_token('~', token_type::bit_not)) {}
		else if (_maybe_token('!', token_type::logical_not)) { // !
			_maybe_token('=', token_type::not_equal); // !=
		} else if (_maybe_token('*', token_type::multiply)) { // *
			_maybe_token('=', token_type::multiply_assign); // *=
		}
		else if (_maybe_token('+', token_type::plus)) { // +
			if (_maybe_token('+', token_type::increment)); // ++
			else _maybe_token('=', token_type::plus_assign); // +=
		}
		else if (_maybe_token('-', token_type::minus)) { // -
			if (_maybe_token('-', token_type::decrement)); // --
			else _maybe_token('=', token_type::minus_assign); // -=
		}
		else if (_maybe_token('<', token_type::less_than)) { // <
			if (_maybe_token('<', token_type::left_shift)) { // <<
				_maybe_token('=', token_type::left_shift_assign); // <<=
			}
			else _maybe_token('=', token_type::less_equal); // <=
		}
		else if (_maybe_token('=', token_type::assign)) { // =
			_maybe_token('=', token_type::equal); // ==
		}
		else if (_maybe_token('>', token_type::greater_than)) { // >
			if (_maybe_token('>', token_type::right_shift)) { // >>
				if (_maybe_token('>', token_type::unsigned_right_shift)) { // >>>
					_maybe_token('=', token_type::unsigned_right_shift_assign); // >>>=
				}
				else _maybe_token('=', token_type::right_shift_assign); // >>=
			}
			else _maybe_token('=', token_type::greater_equal); // >=
		}
		else if (_maybe_token('%', token_type::modulo)) { // %
			_maybe_token('=', token_type::modulo_assign); // %=
		}
		else if (_maybe_token('&', token_type::bit_and)) { // &
			if (_maybe_token('&', token_type::logical_and)); // &&
			else _maybe_token('=', token_type::bit_and_assign); // &=
		}
		else if (_maybe_token('^', token_type::bit_xor)) { // ^
			_maybe_token('=', token_type::bit_xor_assign); // ^=
		}
		else if (_maybe_token('|', token_type::bit_or)) { // |
			if (_maybe_token('|', token_type::logical_or)); // ||
			else _maybe_token('=', token_type::bit_or_assign); // |=
		}
		else {
			throw lexical_error();
		}

		// set line and column
		_current_token.set_line(line);
		_current_token.set_column(column);
	}

	template<typename InIterT, typename StrPoolT>
	void lexer<InIterT, StrPoolT>::_consume_block_comment() {
		while (!_current_is_last()) {
			if (_current() == '*' && _next() == '/') {
				_skip();
				return;
			}
			_skip();
		}
	}

	template<typename InIterT, typename StrPoolT>
	void lexer<InIterT, StrPoolT>::_scan_identifier() {
		auto buffer = std::string{};

		do {
			buffer += _current();
			_skip();
		} while (!_current_is_last() && std::isalnum(_current()));

		const auto symbol = _id_pool.normalize(buffer);
		_current_token = token::create_identifier(symbol);
	}

	template<typename InIterT, typename StrPoolT>
	void lexer<InIterT, StrPoolT>::_scan_integer() {
		int32_t number = 0;
		int32_t old;
		bool overflow = false;

		do {
			old = number;
			number = number * 10 + (_current() - '0');
			if (old > number) {
				// overflow..
				overflow = true;
			}
			_skip();
		} while (std::isdigit(_current()));

		// overflow, or a alpha char apears somewhere => error
		if (std::isalpha(_current()) || overflow) {
			throw lexical_error{};
			return;
		}

		_current_token = token::create_integer_literal((uint32_t) number);
	}

	template<typename InIterT, typename StrPoolT>
	bool lexer<InIterT, StrPoolT>::_current_is_last() {
		return _current_it == _last_it;
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
