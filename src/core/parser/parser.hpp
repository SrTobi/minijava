/**
 * @file parser.hpp
 *
 * @brief
 *     Parser for the MiniJava programming language.
 *
 */

#pragma once

#include <string>
#include <stdexcept>

#include "lexer/token.hpp"
#include "parser/ast.hpp"
#include "parser/ast_factory.hpp"
#include "position.hpp"
#include "source_error.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Exception used to report syntacitcal errors from within the parser.
	 *
	 */
	struct syntax_error: source_error
	{

		/**
		 * @brief
		 *     Creates a new exception object with a generic error message and
		 *     no source location information.
		 *
		 */
		syntax_error();

		/**
		 * @brief
		 *     Creates a new exception object with a custom error message and
		 *     optional source location information.
		 *
		 * @param msg
		 *     custom error message
		 *
		 * @param pos
		 *     parser-defined error location
		 *
		 */
		syntax_error(const std::string& msg, minijava::position pos = {});
	};  // struct syntax_error


	/**
	 * @brief
	 *     Parses a sequence of tokens as a MiniJava program.
	 *
	 * @tparam InIterT
	 *     input iterator type of the token iterator
	 *
	 * @param first
	 *     iterator pointing at the first token of the program
	 *
	 * @param last
	 *     iterator pointing after the last token of the program
	 *
	 * @param factory
	 *     factory to create AST nodes
	 *
	 * @returns
	 *     AST for the given program
	 *
	 * @throws syntax_error
	 *     if the token sequence `[first, last)` is not a syntactical correct
	 *     MiniJava program
	 *
	 */
	template<typename InIterT>
	std::unique_ptr<ast::program> parse_program(InIterT first, InIterT last, ast_factory& factory);

	/**
	 * @brief
	 *     Parses a sequence of tokens as a MiniJava program.
	 *
	 * This function is a convenience overload that uses a newly
	 * default-constructed `ast_factory`.
	 *
	 * @tparam InIterT
	 *     input iterator type of the token iterator
	 *
	 * @param first
	 *     iterator pointing at the first token of the program
	 *
	 * @param last
	 *     iterator pointing after the last token of the program
	 *
	 * @returns
	 *     AST for the given program
	 *
	 * @throws syntax_error
	 *     if the token sequence `[first, last)` is not a syntactical correct
	 *     MiniJava program
	 *
	 */
	template<typename InIterT>
	std::unique_ptr<ast::program> parse_program(InIterT first, InIterT last);

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_PARSER_PARSER_HPP
#include "parser/parser.tpp"
#undef MINIJAVA_INCLUDED_FROM_PARSER_PARSER_HPP
