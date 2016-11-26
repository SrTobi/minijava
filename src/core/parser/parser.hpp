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


namespace minijava
{

	/**
	 * @brief
	 *     Exception used to report syntacitcal errors from within the parser.
	 *
	 */
	struct syntax_error: std::runtime_error
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
		 *     no source location information.
		 *
		 * @param msg
		 *     custom error message
		 *
		 */
		syntax_error(const std::string msg);

		/**
		 * @brief
		 *     Creates a new exception object with a custom error message and
		 *     source location information.
		 *
		 * @param tok
		 *     token at the parser-defined error location
		 *
		 * @param msg
		 *     custom error message
		 *
		 */
		syntax_error(const token& tok, const std::string& msg);

		/**
		 * @brief
		 *     `return`s the line number of the parser-defined error location.
		 *
		 * If the error location is unknown, 0 is `return`ed.
		 *
		 * @returns
		 *     line number
		 *
		 */
		std::size_t line() const noexcept;

		/**
		 * @brief
		 *     `return`s the column number of the parser-defined error
		 *     location.
		 *
		 * If the error location is unknown, 0 is `return`ed.
		 *
		 * @returns
		 *     column number
		 *
		 */
		std::size_t column() const noexcept;

	private:

		/** @brief Line number of the parser-defined error token. */
		std::size_t _line{};

		/** @brief Column number of the parser-defined error token. */
		std::size_t _column{};

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
