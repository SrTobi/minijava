#ifndef MINIJAVA_INCLUDED_FROM_LEXER_TOKEN_TYPE_SET_HPP
#error "Never `#include <lexer/token_type_set.tpp>` directly; `#include <lexer/token_type_set.hpp>` instead."
#endif

#include <cassert>


namespace minijava
{

	constexpr token_type_set::token_type_set(const std::initializer_list<token_type> tts)
		: token_type_set{}
	{
		for (const auto tt : tts) {
			add(tt);
		}
	}

	constexpr bool token_type_set::empty() const noexcept
	{
		for (auto i = std::size_t{}; i < word_count; ++i) {
			if (_bits[i] != word_type{0}) {
				return false;
			}
		}
		return true;
	}

	constexpr bool token_type_set::contains(const token_type tt) const
	{
		const auto idx = _get_index(tt);
		const auto word = _bits[idx.first];
		const auto mask = (word_type{1} << idx.second);
		return (word & mask);
	}

	constexpr void token_type_set::add(const token_type tt)
	{
		const auto idx = _get_index(tt);
		const auto mask = (word_type{1} << idx.second);
		_bits[idx.first] |= mask;
	}

	constexpr void token_type_set::remove(const token_type tt)
	{
		const auto idx = _get_index(tt);
		const auto mask = (word_type{1} << idx.second);
		_bits[idx.first] &= ~mask;
	}

	constexpr std::pair<std::size_t, std::size_t>
	token_type_set::_get_index(const token_type tt)
	{
		const auto idx = index(tt);
		assert(idx < total_token_type_count);
		// We would use `std::div` here if only it were `constexpr`...
		const auto outer_idx = idx / word_bits;
		const auto inner_idx = idx % word_bits;
		return {outer_idx, inner_idx};
	}

	constexpr token_type_set& token_type_set::operator|=(const token_type_set& other) noexcept
	{
		for (auto i = std::size_t{}; i < word_count; ++i) {
			_bits[i] |= other._bits[i];
		}
		return *this;
	}

	constexpr token_type_set operator|(const token_type_set& lhs, const token_type_set& rhs) noexcept
	{
		auto copy = lhs;
		copy |= rhs;
		return copy;
	}

	constexpr token_type_set& token_type_set::operator&=(const token_type_set& other) noexcept
	{
		for (auto i = std::size_t{}; i < word_count; ++i) {
			_bits[i] &= other._bits[i];
		}
		return *this;
	}

	constexpr token_type_set operator&(const token_type_set& lhs, const token_type_set& rhs) noexcept
	{
		auto copy = lhs;
		copy &= rhs;
		return copy;
	}

	constexpr bool token_type_set::equal(const token_type_set& lhs, const token_type_set& rhs) noexcept
	{
		for (auto i = std::size_t{}; i < word_count; ++i) {
			if (lhs._bits[i] != rhs._bits[i]) {
				return false;
			}
		}
		return true;
	}

	constexpr bool operator==(const token_type_set& lhs, const token_type_set& rhs) noexcept
	{
		return token_type_set::equal(lhs, rhs);
	}

	constexpr bool operator!=(const token_type_set& lhs, const token_type_set& rhs) noexcept
	{
		return !token_type_set::equal(lhs, rhs);
	}

}  // namespace minijava
