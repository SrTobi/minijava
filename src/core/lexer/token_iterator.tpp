#ifndef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_ITERATOR_HPP
#error "Never `#include <lexer/token_iterator.tpp>` directly; `#include <token_iterator.hpp>` instead."
#endif

#include <cassert>


namespace minijava
{

	template<typename LexerT>
	token_iterator<LexerT>::token_iterator()
	{
	}

	template<typename LexerT>
	token_iterator<LexerT>::token_iterator(lexer_type& lexer) : _lexer(&lexer)
	{
	}

	template<typename LexerT>
	typename token_iterator<LexerT>::reference token_iterator<LexerT>::operator*()
	{
		assert(_lexer);
		return _lexer->current_token();
	}

	template<typename LexerT>
	typename token_iterator<LexerT>::pointer token_iterator<LexerT>::operator->()
	{
		assert(_lexer);
		return &(_lexer->current_token());
	}

	template<typename LexerT>
	token_iterator<LexerT>& token_iterator<LexerT>::operator++()
	{
		assert(_lexer);
		if (_lexer->current_token_is_eof()) {
			_lexer = nullptr;
		} else {
			_lexer->advance();
		}
		return *this;
	}

	template<typename LexerT>
	bool token_iterator<LexerT>::equal(const token_iterator<LexerT>& lhs,
									   const token_iterator<LexerT>& rhs)
	{
		assert(!lhs._lexer || !rhs._lexer || lhs._lexer == rhs._lexer);
		return (lhs._lexer == rhs._lexer);
	}

	template<typename LexerT>
	bool operator==(const token_iterator<LexerT>& lhs,
					const token_iterator<LexerT>& rhs)
	{
		return token_iterator<LexerT>::equal(lhs, rhs);
	}

	template<typename LexerT>
	bool operator!=(const token_iterator<LexerT>& lhs,
					const token_iterator<LexerT>& rhs)
	{
		return !token_iterator<LexerT>::equal(lhs, rhs);
	}

	template <typename LexerT>
	token_iterator<LexerT> token_begin(LexerT& lexer)
	{
		return token_iterator<LexerT>{lexer};
	}

	template <typename LexerT>
	token_iterator<LexerT> token_end(LexerT&)
	{
		return token_iterator<LexerT>{};
	}

}  // namespace minijava
