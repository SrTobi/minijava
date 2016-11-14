#ifndef MINIJAVA_INCLUDED_FROM_LEXER_CHARACTER_HPP
#error "Never `#include <lexer/character.tpp>` directly; `#include <lexer/character.hpp>` instead."
#endif

#include <bitset>
#include <cstddef>


namespace minijava
{

	namespace detail
	{

		constexpr int ascii_min = 0x00;

		constexpr int ascii_max = 0x7f;

		using ascii_bs_type = std::bitset<ascii_max - ascii_min + 1>;

		constexpr bool is_ascii(const int c) noexcept
		{
			return ((c >= ascii_min) && (c <= ascii_max));
		}

		inline ascii_bs_type make_ascii_bs(const char *const chars) noexcept
		{
			auto bs = ascii_bs_type{};
			for (auto it = chars; *it != '\0'; ++it) {
				const std::size_t idx = static_cast<unsigned char>(*it);
				bs[idx] = true;
			}
			return bs;
		}

	}  // namespace detail

	inline bool is_space(const int c) noexcept
	{
		static const auto bs = detail::make_ascii_bs("\t\n\r ");
		return detail::is_ascii(c) && bs[static_cast<std::size_t>(c)];
	}

	inline bool is_digit(const int c) noexcept
	{
		static const auto bs = detail::make_ascii_bs("0123456789");
		return detail::is_ascii(c) && bs[static_cast<std::size_t>(c)];
	}

	inline bool is_word_head(const int c) noexcept
	{
		static const auto bs = detail::make_ascii_bs(
			"_"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
		);
		return detail::is_ascii(c) && bs[static_cast<std::size_t>(c)];
	}

	inline bool is_word_tail(const int c) noexcept
	{
		static const auto bs = detail::make_ascii_bs(
			"_"
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
		);
		return detail::is_ascii(c) && bs[static_cast<std::size_t>(c)];
	}

}  // namespace minijava
