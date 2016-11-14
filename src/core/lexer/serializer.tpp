#ifndef MINIJAVA_INCLUDED_FROM_PARSER_SERIALIZER_HPP
#error "Never `#include <parser/serializer.tpp>` directly; `#include <parser/serializer.hpp>` instead."
#endif


namespace minijava
{

	namespace detail
	{

		int serialize_next_token(std::ostream& os,
								 const token& tok,
								 int indent,
								 token_type prev);

	}

	template <typename InIterT>
	void // std::enable_if_t<std::is_same<token, typename std::iterator_traits<InIterT>::value_type>{}>
	pretty_print(std::ostream& os, const InIterT first, const InIterT last)
	{
		auto indent = 0;
		auto prev = token_type::eof;
		for (auto it = first; it != last; ++it) {
			indent = detail::serialize_next_token(os, *it, indent, prev);
			prev = it->type();
		}
	}

}  // namespace minijava
