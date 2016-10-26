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
		if (_next == _last) {
			_current_token = token::create(token_type::eof);
			return;
		}


		// skip whitespaces
		while (std::isspace(current())) {
			if (current() == '\n') {
				_line++;
				_column=0;
			}
			skip();
		}

		// store token start position
		auto line = _line;
		auto column = _column;

		auto c = current();
		if (std::isalpha(c)) {
			// scan identifier
			scan_identifier();
		} else if (std::isdigit(c)) {
			// scan digit
			scan_integer();
		} else if (c == '/') {
			auto next_char = next();
			// comment?
			if (next_char == '*') {
				// skip comment and run advance again
				consume_block_comment();
				advance();
				return;
			} else if (next_char == '=') {
				// /= token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// / token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '[') {
			// [ token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == ']') {
			// ] token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == '(') {
			// ( token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == ')') {
			// ) token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == '{') {
			// { token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == '}') {
			// } token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == '!') {
			auto next_char = next();
			if (next_char == '=') {
				// != token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// ! token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '*') {
			auto next_char = next();
			if (next_char == '=') {
				// *= token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// * token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '+') {
			auto next_char = next();
			if (next_char == '+') {
				// ++ token
				_current_token = token::create(token_type::unknown);
				skip();
			} else if (next_char == '=') {
				// += token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// + token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '-') {
			auto next_char = next();
			if (next_char == '-') {
				// -- token
				_current_token = token::create(token_type::unknown);
				skip();
			} else if (next_char == '=') {
				// -= token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// - token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '.') {
			// . token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == ':') {
			// : token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == ';') {
			// ; token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == '<') {
			auto next_char = next();
			if (next_char == '<') {
				auto n2 = next();
				if (n2 == '=') {
					// <<= token
					_current_token = token::create(token_type::unknown);
					skip();
				} else {
					// << token
					_current_token = token::create(token_type::unknown);
				}
			} else if (next_char == '=') {
				// <= token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// < token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '=') {
			auto next_char = next();
			if (next_char == '=') {
				// == token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// = token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '>') {
			auto next_char = next();
			if (next_char == '>') {
				auto n2 = next();
				if (n2 == '>') {
					auto n3 = next();
					if (n3 == '=') {
						// >>>= token
						_current_token = token::create(token_type::unknown);
						skip();
					} else {
						// >>> token
						_current_token = token::create(token_type::unknown);
					}
				} else if (n2 == '=') {
					// >>= token
					_current_token = token::create(token_type::unknown);
					skip();
				} else {
					// >> token
					_current_token = token::create(token_type::unknown);
				}
			} else if (next_char == '=') {
				// >= token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// > token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '?') {
			// ? token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == '%') {
			auto next_char = next();
			if (next_char == '=') {
				// %= token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// % token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '&') {
			auto next_char = next();
			if (next_char == '&') {
				// && token
				_current_token = token::create(token_type::unknown);
				skip();
			} else if (next_char == '=') {
				// &= token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// & token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '^') {
			auto next_char = next();
			if (next_char == '^') {
				// ^ token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// ^ token
				_current_token = token::create(token_type::unknown);
			}
		} else if (c == '~') {
			// ~ token
			_current_token = token::create(token_type::unknown);
			skip();
		} else if (c == '|') {
			auto next_char = next();
			if (next_char == '|') {
				// || token
				_current_token = token::create(token_type::unknown);
				skip();
			} else if (next_char == '=') {
				// |= token
				_current_token = token::create(token_type::unknown);
				skip();
			} else {
				// | token
				_current_token = token::create(token_type::unknown);
			}
		} else {
			throw lexical_error();
		}

		// set line and column
		_current_token.set_line(line);
		_current_token.set_column(column);
	}

	template<typename InIterT, typename StrPoolT>
	void lexer<InIterT, StrPoolT>::consume_block_comment() {
		while (_next != _last) {
			if (current() == '*' && next() == '/') {
				skip();
				return;
			}
			skip();
		}
	}

	template<typename InIterT, typename StrPoolT>
	void lexer<InIterT, StrPoolT>::scan_identifier() {
		auto buffer = std::string{};

		do {
			buffer += current();
			skip();
		} while (_next != _last && std::isalnum(current()));

		const auto symbol = _id_pool.normalize(buffer);
		_current_token = token::create_identifier(symbol);
	}

	template<typename InIterT, typename StrPoolT>
	void lexer<InIterT, StrPoolT>::scan_integer() {
		int32_t number = 0;
		int32_t old;
		bool overflow = false;

		do {
			old = number;
			number = number * 10 + ((int)current() - '0');
			if (old > number) {
				// overflow..
				overflow = true;
			}
			skip();
		} while (std::isdigit(current()));

		// overflow, or a alpha char apears somewhere => error
		if (std::isalpha(current()) || overflow) {
			throw lexical_error{};
			return;
		}

		_current_token = token::create_integer_literal(number);
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
