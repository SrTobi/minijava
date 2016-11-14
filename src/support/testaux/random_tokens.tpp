#pragma once

#include <cassert>
#include <cstring>
#include <random>

#include "lexer/keyword.hpp"
#include "lexer/token_type.hpp"


namespace testaux
{

	namespace detail
	{

		constexpr auto id_head_chars =
			"_"
			"ABCDEFGHILKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		constexpr auto id_tail_chars =
			"_"
			"0123456789"
			"ABCDEFGHILKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		template <typename RndEngT>
		minijava::token_type get_random_token(RndEngT& engine, const minijava::token_category cat)
		{
			// This algorithm might seem hilariously inefficient at first sight
			// but on a second thought, we only use it to generate keyword and
			// punctuation tokens for which we have approximately a 50 % chance
			// of hitting one in the array so the expectation value for the
			// number of iterations is about 2 which seems to be justified by
			// the gained simplicity.
			auto idxdist = std::uniform_int_distribution<std::size_t>{
				0,
				minijava::total_token_type_count - 1
			};
			while (true) {
				const auto idx = idxdist(engine);
				const auto tt = minijava::all_token_types()[idx];
				if (category(tt) == cat) {
					return tt;
				}
			}
		}

	}  // namespace detail

	template <typename RndEngT>
	std::string get_random_identifier(RndEngT& engine, const std::size_t length)
	{
		assert(length >= 1);
		const auto maxhead = std::strlen(detail::id_head_chars) - 1;
		const auto maxtail = std::strlen(detail::id_tail_chars) - 1;
		auto headidxdist = std::uniform_int_distribution<std::size_t>{0, maxhead};
		auto tailidxdist = std::uniform_int_distribution<std::size_t>{0, maxtail};
		auto buffer = std::string{};
		buffer.reserve(length);
		do {
			buffer.clear();
			buffer.push_back(detail::id_head_chars[headidxdist(engine)]);
			while (buffer.length() < length) {
				buffer.push_back(detail::id_tail_chars[tailidxdist(engine)]);
			}
		} while (minijava::classify_word(buffer) != minijava::token_type::identifier);
		return buffer;
	}

	template <typename RndEngT>
	std::string get_random_identifier(RndEngT& engine, const double p)
	{
		assert((p > 0.0) && (p < 1.0));
		auto lendist = std::geometric_distribution<std::size_t>{p};
		const auto length = 1 + lendist(engine);
		return get_random_identifier(engine, length);
	}

	template <typename RndEngT>
	std::string get_random_identifier(RndEngT& engine, const std::size_t minlen, const std::size_t maxlen)
	{
		assert(minlen >= 1);
		assert(minlen <= maxlen);
		auto lendist = std::uniform_int_distribution<std::size_t>{minlen, maxlen};
		const auto length = lendist(engine);
		return get_random_identifier(engine, length);
	}

	template <typename RndEngT>
	std::string get_random_integer_literal(RndEngT& engine, const std::size_t length)
	{
		assert(length >= 1);
		auto headdist = std::uniform_int_distribution<int>{'1', '9'};
		auto taildist = std::uniform_int_distribution<int>{'0', '9'};
		auto buffer = std::string{};
		buffer.reserve(length);
		if (length > 1) {
			buffer.push_back(static_cast<char>(headdist(engine)));
		}
		while (buffer.length() < length) {
			buffer.push_back(static_cast<char>(taildist(engine)));
		}
		return buffer;
	}

	template <typename RndEngT>
	std::string get_random_integer_literal(RndEngT& engine, const double p)
	{
		assert((p > 0.0) && (p < 1.0));
		auto lendist = std::geometric_distribution<std::size_t>{p};
		const auto length = 1 + lendist(engine);
		return get_random_integer_literal(engine, length);
	}

	template <typename RndEngT>
	std::string get_random_integer_literal(RndEngT& engine, const std::size_t minlen, const std::size_t maxlen)
	{
		assert(minlen >= 1);
		assert(minlen <= maxlen);
		auto lendist = std::uniform_int_distribution<std::size_t>{minlen, maxlen};
		const auto length = lendist(engine);
		return get_random_integer_literal(engine, length);
	}

	template <typename RndEngT>
	minijava::token_type get_random_keyword(RndEngT& engine)
	{
		return detail::get_random_token(engine, minijava::token_category::keyword);
	}

	template <typename RndEngT>
	minijava::token_type get_random_punctuation(RndEngT& engine)
	{
		return detail::get_random_token(engine, minijava::token_category::punctuation);
	}

}  // namespace testaux
