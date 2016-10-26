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
			constexpr auto r = detail::get_token_type_info_table();
			static_assert(r.second - r.first == iseq.size(), "wrong array size");
			std::array<token_type, 102> array = {{ (r.first[Is].type)... }};
			return array;
		}

	}


	const std::array<token_type, 102>& all_token_types() noexcept
	{
		using idxseq_type = std::make_index_sequence<102>;
		static constexpr auto array = make_all_token_types_array(idxseq_type{});
		return array;
	}


	std::ostream& operator<<(std::ostream& os, const token_type tt)
	{
		using raw_type = std::underlying_type_t<token_type>;
		if (const auto text = name(tt)) {
			os << text;
		} else {
			os << "token_type(" << static_cast<raw_type>(tt) << ")";
		}
		return os;
	}

}  // namespace minijava
