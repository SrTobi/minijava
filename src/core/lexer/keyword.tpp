#ifndef MINIJAVA_INCLUDED_FROM_LEXER_KEYWORD_HPP
#error "Never `#include <lexer/keyword.tpp>` directly; `#include <lexer/keyword.hpp>` instead."
#endif

#include <cassert>

#include "lexer/character.hpp"


namespace minijava
{
	namespace detail
	{

		// Computes the hash for the  bytes in the range `[first, last)` using
		// the perfect hash function for the MiniJava keywords.
		constexpr std::size_t hash_word(const char*const first, const char*const last) noexcept
		{
			auto hash = 0u;
			for (auto it = first; it != last; ++it) {
				const auto idx = std::size_t{static_cast<unsigned char>(*it)};
				assert(idx < sizeof(keyword_pearson_table) / sizeof(keyword_pearson_table[0]));
				hash ^= keyword_pearson_table[idx];
			}
			return hash;
		}

		// Tests whether the non-NUL bytes in the range `[data, data + length)`
		// match the NUL-terminated character string `target`.
		constexpr bool true_match(const char*const target,
		                          const char*const data,
		                          const std::size_t length) noexcept
		{
			for (std::size_t i = 0; i < length; ++i) {
				if (target[i] != data[i]) {
					return false;
				}
			}
			return (target[length] == '\0');
		}

		constexpr token_type classify_word(const char*const data, const std::size_t length) noexcept
		{
			if ((length < keyword_min_length) || (length > keyword_max_length)) {
				return token_type::identifier;
			}
			const auto idx = hash_word(data, data + length);
			assert(idx < sizeof(keyword_lookup_table) / sizeof(keyword_lookup_table[0]));
			const auto match = keyword_lookup_table[idx];
			if (match == token_type::identifier) {
				return token_type::identifier;
			}
			if (!true_match(name(match), data, length)) {
				return token_type::identifier;
			}
			return match;
		}

	}  // namespace detail


	inline token_type classify_word(const std::string& word)
	{
		assert(is_word(word));
		return detail::classify_word(word.data(), word.length());
	}

}  // namespace minijava
