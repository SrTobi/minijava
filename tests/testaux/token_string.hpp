/**
 * @file token_string.hpp
 *
 * @brief
 *     helper functions for creating tokens in tests
 *
 */

#pragma once

#include <string>

#include "lexer/token.hpp"
#include "lexer/token_type.hpp"
#include "symbol_pool.hpp"


namespace testaux
{

	/**
	 * @brief
	 *     string "tagged" with a token type
	 *
	 * @tparam TokenType
	 *     token type the string represents
	 *
	 */
	template <minijava::token_type TokenType>
	struct tagged_string
	{
		std::string s{};
	};

	/**
	 * @brief
	 *     creates a string representing an identifier
	 *
	 * @param text
	 *     name of the identifier
	 *
	 * @return
	 *     tagged string
	 *
	 */
	auto id(std::string text)
	{
		return tagged_string<minijava::token_type::identifier> {std::move(text)};
	}

	/**
	 * @brief
	 *     creates a string representing an integer literal
	 *
	 * @param text
	 *     integer literal
	 *
	 * @return
	 *     tagged string
	 *
	 */
	auto lit(std::string text)
	{
		return tagged_string<minijava::token_type::integer_literal> {std::move(text)};
	}

	/**
	 * @brief
	 *     creates an appropriate token from either a tagged identifier string
	 *
	 * @param text
	 *     tagged string representing the identifier
	 *
	 * @returns
	 *     identifier token
	 *
	 */
	minijava::token make_token(minijava::symbol_pool<>& pool,
	                            const tagged_string<minijava::token_type::identifier>& text)
	{
		return minijava::token::create_identifier(pool.normalize(text.s));
	}

	/**
	 * @brief
	 *     creates an appropriate token from either a tagged integer literal string
	 *
	 * @param text
	 *     tagged string representing the integer literal
	 *
	 * @returns
	 *     integer literal token
	 *
	 */
	minijava::token make_token(minijava::symbol_pool<>& pool,
	                            const tagged_string<minijava::token_type::integer_literal>& text)
	{
		return minijava::token::create_integer_literal(pool.normalize(text.s));
	}

	/**
	 * @brief
	 *     creates an appropriate token of the given type
	 *
	 * @param type
	 *     requested token type
	 *
	 * @returns
	 *     token of the given type
	 *
	 */
	minijava::token make_token(minijava::symbol_pool<>& pool,
	                            const minijava::token_type type)
	{
		(void) pool;
		return minijava::token::create(type);
	}

}  // namespace testaux
