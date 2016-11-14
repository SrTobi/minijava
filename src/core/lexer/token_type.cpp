#include "token_type.hpp"

#include <ostream>
#include <type_traits>
#include <utility>


namespace minijava
{

	namespace /* anonymous */
	{

		template <std::size_t... Is>
		constexpr auto make_all_token_types_array(std::index_sequence<Is...> iseq) noexcept
		{
			constexpr auto first = std::begin(detail::token_type_info_table);
			constexpr auto last = std::end(detail::token_type_info_table);
			static_assert(total_token_type_count == iseq.size(), "");
			static_assert(total_token_type_count == last - first, "");
			std::array<token_type, total_token_type_count> array = {
				{ (detail::token_type_info_table[Is].first)... }
			};
			return array;
		}

	}


	const std::array<token_type, total_token_type_count>& all_token_types() noexcept
	{
		using idxseq_type = std::make_index_sequence<total_token_type_count>;
		static constexpr auto array = make_all_token_types_array(idxseq_type{});
		return array;
	}


	std::ostream& operator<<(std::ostream& os, const token_type tt)
	{
		using raw_t = std::underlying_type_t<token_type>;
		if (const auto text = name(tt)) {
			os << text;
		} else {
			os << "token_type(" << (0U + static_cast<raw_t>(tt)) << ")";
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const token_category cat)
	{
		using raw_t = std::underlying_type_t<token_category>;
		if (const auto text = name(cat)) {
			os << text;
		} else {
			os << "token_category(" << (0U + static_cast<raw_t>(cat)) << ")";
		}
		return os;
	}

}  // namespace minijava
