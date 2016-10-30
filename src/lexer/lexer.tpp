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


	}  // namespace detail


	// This `struct` has a collection of `static` member functions that are
	// used to implement the `lexer`.  By defining them in this nested `class`,
	// we can move them out of the public header file and into this
	// implementation file.  Since `lexer_impl` is a nested `struct` of
	// `lexer`, its member functions can access all `private` members of
	// `lexer`.  The only difference to an ordinary member function is that we
	// don't have a `this` pointer so all functions get the `lexer` object
	// passed as their first argument.
	template <typename InIterT, typename SymPoolT, typename AllocT>
	struct lexer<InIterT, SymPoolT, AllocT>::lexer_impl
	{

		// Type alias for the outer `class`.
		using lexer_type = lexer<InIterT, SymPoolT, AllocT>;

		// Like the public `lexer::advance` function but has to be called in a
		// loop until it succeeds.
		static bool do_advance(lexer_type& lex)
		{
			// TODO: Set line and column attributes of the scanned token.
			const auto c = lexer_impl::skip_white_space(lex);
			if (c < 0) {
				lex._current_token = token::create(token_type::eof);
			} else if (detail::isidhead(c)) {
				scan_identifier(lex);
			} else if (detail::isdigit(c)) {
				scan_integer_literal(lex);
			} else if (c == '/') {
				const auto after = next(lex);
				if (after == '*') {
					// Skip '*' from opening '/*' to not confuse it as part of closing '*/'.
					next(lex);
					skip_block_comment(lex);
					return false;
				} else if (after == '=') {
					lex._current_token = token::create(token_type::divides_assign);
					next(lex);
				} else {
					lex._current_token = token::create(token_type::divides);
				}
			} else if (maybe_token(lex, '[', token_type::left_bracket)) {
			} else if (maybe_token(lex, ']', token_type::right_bracket)) {
			} else if (maybe_token(lex, '(', token_type::left_paren)) {
			} else if (maybe_token(lex, ')', token_type::right_paren)) {
			} else if (maybe_token(lex, '{', token_type::left_brace)) {
			} else if (maybe_token(lex, '}', token_type::right_brace)) {
			} else if (maybe_token(lex, '.', token_type::dot)) {
			} else if (maybe_token(lex, ',', token_type::comma)) {
			} else if (maybe_token(lex, ':', token_type::colon)) {
			} else if (maybe_token(lex, ';', token_type::semicolon)) {
			} else if (maybe_token(lex, '?', token_type::conditional)) {
			} else if (maybe_token(lex, '~', token_type::bit_not)) {
			} else if (maybe_token(lex, '!', token_type::logical_not)) {
				maybe_token(lex, '=', token_type::not_equal);
			} else if (maybe_token(lex, '*', token_type::multiply)) {
				maybe_token(lex, '=', token_type::multiply_assign);
			} else if (maybe_token(lex, '+', token_type::plus)) {
				if (!maybe_token(lex, '+', token_type::increment)) {
					maybe_token(lex, '=', token_type::plus_assign);
				}
			} else if (maybe_token(lex, '-', token_type::minus)) {
				if (!maybe_token(lex, '-', token_type::decrement)) {
					maybe_token(lex, '=', token_type::minus_assign);
				}
			} else if (maybe_token(lex, '<', token_type::less_than)) {
				if (maybe_token(lex, '<', token_type::left_shift)) {
					maybe_token(lex, '=', token_type::left_shift_assign);
				} else {
					maybe_token(lex, '=', token_type::less_equal);
				}
			} else if (maybe_token(lex, '=', token_type::assign)) {
				maybe_token(lex, '=', token_type::equal);
			} else if (maybe_token(lex, '>', token_type::greater_than)) {
				if (maybe_token(lex, '>', token_type::right_shift)) {
					if (maybe_token(lex, '>', token_type::unsigned_right_shift)) {
						maybe_token(lex, '=', token_type::unsigned_right_shift_assign);
					} else {
						maybe_token(lex, '=', token_type::right_shift_assign);
					}
				} else {
					maybe_token(lex, '=', token_type::greater_equal);
				}
			} else if (maybe_token(lex, '%', token_type::modulo)) {
				maybe_token(lex, '=', token_type::modulo_assign);
			} else if (maybe_token(lex, '&', token_type::bit_and)) {
				if (!maybe_token(lex, '&', token_type::logical_and)) {
					maybe_token(lex, '=', token_type::bit_and_assign);
				}
			} else if (maybe_token(lex, '^', token_type::bit_xor)) {
				maybe_token(lex, '=', token_type::bit_xor_assign);
			} else if (maybe_token(lex, '|', token_type::bit_or)) {
				if (!maybe_token(lex, '|', token_type::logical_or)) {
					maybe_token(lex, '=', token_type::bit_or_assign);
				}
			} else {
				throw lexical_error{};
			}
			return true;
		}

		// If the current input character is `c`, sets `_current_token` to that
		// token and advances the input iterator of `lex`.  Otherwise, the
		// function has no effect.  `return`s whether the token was scanned.
		static bool maybe_token(lexer_type& lex, const int c, const token_type tt)
		{
			if (current(lex) != c) {
				return false;
			}
			lex._current_token = token::create(tt);
			next(lex);
			return true;
		}

		// Scans an identifier or keyword (both referred to as "word" in the
		// following) token.  On entry, `current(lex)` must be a valid begin of
		// a word.  Otherwise, the behavior is undefined.  `lex._current_token`
		// is set to the scanned word and the input iterator is advanced to the
		// character past the last character that was part of the scanned
		// token.
		static void scan_identifier(lexer_type& lex)
		{
			lex._lexbuf.clear();
			auto c = current(lex);
			assert(detail::isidhead(c));
			do {
				lex._lexbuf.push_back(static_cast<char>(c));
				c = next(lex);
			} while (detail::isidtail(c));
			const auto tt = classify_word(lex._lexbuf);
			if (tt == token_type::identifier) {
				const auto lexval = lex._id_pool.normalize(lex._lexbuf);
				lex._current_token = token::create_identifier(lexval);
			} else {
				assert(category(tt) == token_category::keyword);
				lex._current_token = token::create(tt);
			}
		}

		// Scans an integer literal token.  On entry, `current(lex)` must be a
		// valid begin of an integer literal.  Otherwise, the behavior is
		// undefined.  `lex._current_token` is set to the scanned integer
		// literal and the input iterator is advanced to the character past the
		// last character that was part of the scanned token.
		static void scan_integer_literal(lexer_type& lex)
		{
			lex._lexbuf.clear();
			auto c = current(lex);
			assert(detail::isdigit(c));
			do {
				lex._lexbuf.push_back(static_cast<char>(c));
				c = next(lex);
			} while (detail::isdigit(c));
			if ((lex._lexbuf.front() == '0') && (lex._lexbuf.size() > 1)) {
				throw lexical_error{"Invalid integer literal: leading zeros are not allowed"};
			}
			const auto lexval = lex._lit_pool.normalize(lex._lexbuf);
			lex._current_token = token::create_integer_literal(lexval);
		}

		// Skips over a block /* ... */ comment.  The input iterator is
		// advanced to the first character after the next '*/' and its value is
		// `return`ed.  This means that on entry, the current character must
		// already be the character following the '/*' that opened the
		// block-comment or it will be mis.interpreted as part of a potential
		// closing '*/' sequence.  If input ends before '*/' was seen, a
		// `lexical_error` is `throw`n.
		static int skip_block_comment(lexer_type& lex)
		{
			enum dfa_state {q0, q1, q2};
			auto state = q0;
			for (auto c = current(lex); true; c = next(lex)) {
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

		// Skips over white-space.  The input iterator is advanced to the next
		// character that is not considered white-space.  The value of that
		// character is `return`ed.  If the current input character is not
		// white-space, then this function has no effect and simply `return`s
		// `current(lex)`.
		static int skip_white_space(lexer_type& lex) noexcept
		{
			auto c = current(lex);
			while (detail::isspace(c)) {
				c = next(lex);
			}
			return c;
		}

		// `return`s the current input character.  If the input sequence is not
		// yet exhausted, the current character is `return`ed.  Otherwise, if
		// if the end of the input was already reached, -1 is `return`ed.
		static int current(const lexer_type& lex) noexcept
		{
			return (lex._current_it != lex._last_it)
				? static_cast<unsigned char>(*lex._current_it)
				: -1;
		}

		// `return`s the next input character.  If the input sequence is
		// already exhausted on entry, this function has no effect.  Otherwise,
		// the input iterator is advanced and the line and column numbers
		// updated.  Then `current(lex)` is `return`ed.
		static int next(lexer_type& lex) noexcept
		{
			if (lex._current_it == lex._last_it) {
				return -1;
			}
			++lex._current_it;
			const auto c = current(lex);
			if (c == '\n') {
				lex._line += 1;
				lex._column = 0;
			} else {
				lex._column += 1;
			}
			return c;
		}

	};  // struct lexer_impl


	template<typename InIterT, typename SymPoolT, typename AllocT>
	lexer<InIterT, SymPoolT, AllocT>::lexer(
		const InIterT first, const InIterT last,
		SymPoolT& id_pool, SymPoolT& lit_pool,
		const AllocT& alloc) :
		_current_token{token::create(token_type::eof)},
		_current_it{first}, _last_it{last},
		_id_pool{id_pool}, _lit_pool{lit_pool},
		_line{0}, _column{0},
		_lexbuf{alloc}
	{
		advance();
	}

	template<typename InIterT, typename SymPoolT, typename AllocT>
	const token& lexer<InIterT, SymPoolT, AllocT>::current_token() const noexcept
	{
		return _current_token;
	}

	template<typename InIterT, typename SymPoolT, typename AllocT>
	bool lexer<InIterT, SymPoolT, AllocT>::current_token_is_eof() const noexcept
	{
		return (current_token().type() == token_type::eof);
	}

	template<typename InIterT, typename SymPoolT, typename AllocT>
	void lexer<InIterT, SymPoolT, AllocT>::advance()
	{
		while (!lexer_impl::do_advance(*this)) {
			// Try again, eh?
		}
	}

	template<typename InIterT, typename SymPoolT, typename AllocT>
	lexer<InIterT, SymPoolT, AllocT>
	make_lexer(
		const InIterT first, const InIterT last,
		SymPoolT& id_pool, SymPoolT& lit_pool,
		const AllocT& alloc)
	{
		return lexer<InIterT, SymPoolT, AllocT>{first, last, id_pool, lit_pool, alloc};
	}

}  // namespace minijava
