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
			auto table = std::map<std::string, token_type>{};
			const auto tti = detail::get_token_type_info_table();
			for (auto it = tti.first; it != tti.second; ++it) {
				if (category(it->first) == token_category::keyword) {
					table[it->second] = it->first;
				}
			}
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
