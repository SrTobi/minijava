#ifndef TESTAUX_INCLUDED_FROM_RANDOM_HPP
#error "Never `#include <testaux/random.tpp>` directly; `#include <testaux/random.hpp>` instead."
#endif

#include <algorithm>
#include <climits>
#include <iterator>
#include <random>


namespace testaux
{

	template <typename RndEngT>
	std::string make_random_string(RndEngT& engine, const std::size_t n)
	{
		// We have to use a distribution of `int`s and then cast back to `char`
		// as instantiating `std::uniform_int_distribution<char>` officially
		// (for reasons beyond my understanding) results in undefined behavior,
		// although it seems to work flawlessly with all standard library
		// implementations I've tried.
		auto dist = std::uniform_int_distribution<int>{CHAR_MIN, CHAR_MAX};
		const auto lambda = [&engine, &dist](){
			return static_cast<char>(dist(engine));
		};
		auto s = std::string{};
		s.reserve(n);
		std::generate_n(std::back_inserter(s), n, lambda);
		return s;
	}

	inline std::string make_random_string(const std::size_t n)
	{
		auto engine = std::default_random_engine{};
		return make_random_string(engine, n);
	}

}  // namespace testaux
