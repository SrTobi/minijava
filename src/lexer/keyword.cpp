#include "keyword.hpp"

#include <algorithm>
#include <iterator>
#include <map>
#include <string>


// TODO @Moritz Klammler: Optimize the hell out of this.

namespace minijava
{

	namespace /* anonymous */
	{

		std::map<std::string, token_type> make_lookup_table()
		{
			auto values = all_token_types();
			const auto last = std::remove_if(
				std::begin(values), std::end(values),
				[](auto tt){ return category(tt) != token_category::keyword; }
			);
			auto table = std::map<std::string, token_type>{};
			std::transform(
				std::begin(values), last, std::inserter(table, std::end(table)),
				[](auto tt){ return std::pair<std::string, token_type>{fancy_name(tt), tt}; }
			);
			return table;
		}

	}

	token_type classify_word(const std::string& word)
	{
		using namespace std::string_literals;
		static const auto keywords = make_lookup_table();
		const auto pos = keywords.find(word);
		return (pos != keywords.cend()) ? pos->second : token_type::identifier;
	}

}  // namespace minijava
