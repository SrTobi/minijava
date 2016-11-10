#pragma once

#include <iosfwd>
#include <iterator>
#include <type_traits>

#include "lexer/token.hpp"


namespace minijava
{

	/**
	 * @brief
	 *     Serializes a token stream into program text.
	 *
	 * White-space is added using a pretty basic heuristic.  The line and
	 * column attributes of the tokens are ignored.
	 *
	 * @tparam InIterT
	 *     input iterator type that provides the token stream
	 *
	 * @param os
	 *     stream to write the program text to
	 *
	 * @param fist
	 *     iterator to the first token of the program
	 *
	 * @param last
	 *     iterator after the last token of the program
	 *
	 */
	template <typename InIterT>
	void // std::enable_if_t<std::is_same<token, typename std::iterator_traits<InIterT>::value_type>{}>
	pretty_print(std::ostream& os, InIterT first, InIterT last);

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_PARSER_SERIALIZER_HPP
#include "serializer.tpp"
#undef MINIJAVA_INCLUDED_FROM_PARSER_SERIALIZER_HPP
