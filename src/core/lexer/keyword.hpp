/**
 * @file keyword.hpp
 *
 * @brief
 *     Keyword classification.
 *
 */

#pragma once

#include "lexer/token_type.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Classifies a word either as one of the keywords or else as
	 *     identifier.
	 *
	 * If `word` is neither a keyword nor a valid identifier, the behavior is
	 * undefined.
	 *
	 * @param word
	 *     word to classify
	 *
	 * @returns
	 *     keyword type or else `token_type::identifier`
	 *
	 */
	token_type classify_word(const std::string& word);

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_LEXER_KEYWORD_HPP
#include "lexer/keyword_pearson.tpp"
#include "lexer/keyword.tpp"
#undef MINIJAVA_INCLUDED_FROM_LEXER_KEYWORD_HPP
