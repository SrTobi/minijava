#include "lexer/character.hpp"

#include <algorithm>
#include <iterator>


namespace minijava
{

	bool is_number(const std::string& s) noexcept
	{
		const auto pred = [](auto&& c){
			return is_digit(static_cast<unsigned char>(c));
		};
		if (s.empty()) {
			return false;
		}
		if (!std::all_of(std::begin(s), std::end(s), pred)) {
			return false;
		}
		if ((s.front() == '0') && (s.length() > 1)) {
			return false;
		}
		return true;
	}

	bool is_word(const std::string& s) noexcept
	{
		const auto pred_head = [](auto&& c){
			return is_word_head(static_cast<unsigned char>(c));
		};
		const auto pred_tail = [](auto&& c){
			return is_word_tail(static_cast<unsigned char>(c));
		};
		if (s.empty()) {
			return false;
		}
		if (!pred_head(s.front())) {
			return false;
		}
		if (!std::all_of(std::next(std::begin(s)), std::end(s), pred_tail)) {
			return false;
		}
		return true;
	}

}  // namespace minijava
