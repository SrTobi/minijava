#ifndef MINIJAVA_INCLUDED_FROM_LEXER_LEXER_HPP
#error "Never `#include <lexer/lexer.tpp>` directly; `#include <lexer.hpp>` instead."
#endif

#include <cctype>

#include "lexer/keyword.hpp"


namespace minijava
{

	namespace detail
	{

		inline bool isidhead(const int c) noexcept {
			return ((c == '_') || std::isalpha(c));
		}

		inline bool isidtail(const int c) noexcept {
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
		_line = 1;
		_column = 1;
	}

	template<typename InIterT, typename SymPoolT>
	const token& lexer<InIterT, SymPoolT>::current_token() const noexcept
	{
		return _current_token;
	}

	template<typename InIterT, typename SymPoolT>
	void lexer<InIterT, SymPoolT>::advance()
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
		if (detail::isidhead(c)) {
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

	template<typename InIterT, typename SymPoolT>
	void lexer<InIterT, SymPoolT>::_consume_block_comment() {
		assert(*_current_it == '*');
		++_current_it;
		auto state = 0;
		while (_current_it != _last_it) {
			if (state == 1) {
				if (*_current_it == '/') {
					++_current_it;
					state = 2;
					break;
				}
				state = 0;
			}
			if (*_current_it == '*') {
				state = 1;
			}
			++_current_it;
		}
		if (state != 2) {
			throw lexical_error{"File ended before '*/' closing comment"};
		}
	}

	template<typename InIterT, typename SymPoolT>
	void lexer<InIterT, SymPoolT>::_scan_identifier()
	{
		auto buffer = std::string{};
		do {
			buffer += *_current_it;
			++_current_it;
		} while ((_current_it != _last_it) && detail::isidtail(*_current_it));
		const auto tt = classify_word(buffer);
		if (tt == token_type::identifier) {
			const auto symbol = _id_pool.normalize(buffer);
			_current_token = token::create_identifier(symbol);
		} else {
			assert(category(tt) == token_category::keyword);
			_current_token = token::create(tt);
		}
	}

	template<typename InIterT, typename SymPoolT>
	void lexer<InIterT, SymPoolT>::_scan_integer()
	{
		assert(std::isdigit(_current()));
		if (_current() == '0') {
			_skip();
			if (!_current_is_last() && std::isdigit(_current())) {
				throw lexical_error{"Invalid integer literal: leading zeros are not allowed"};
			}
			const auto symbol = _lit_pool.normalize("0");
			_current_token = token::create_integer_literal(symbol);
		} else {
			auto buffer = std::string{};
			do {
				buffer += _current();
				_skip();
			} while (!_current_is_last() && std::isdigit(_current()));
			const auto symbol = _lit_pool.normalize(buffer);
			_current_token = token::create_integer_literal(symbol);
		}
	}

	template<typename InIterT, typename SymPoolT>
	bool lexer<InIterT, SymPoolT>::_current_is_last() {
		return _current_it == _last_it;
	}

	template<typename InIterT, typename SymPoolT>
	bool lexer<InIterT, SymPoolT>::current_token_is_eof() const noexcept
	{
		return (current_token().type() == token_type::eof);
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
