#include "keyword.hpp"

#include <map>
#include <string>


namespace minijava
{

	token_type classify_word(const std::string& word)
	{
		// TODO @Moritz Klammler: Complete this function and make it performant.
		using namespace std::string_literals;
		static const auto keywords = std::map<std::string, token_type>{
			{fancy_name(token_type::kw_if),   token_type::kw_if},
			{fancy_name(token_type::kw_else), token_type::kw_else},
			// ...
		};
		const auto pos = keywords.find(word);
		return (pos != keywords.cend()) ? pos->second : token_type::identifier;
	}

}  // namespace minijava
