#include "semantic/semantic.hpp"

namespace minijava
{

	bool semantic_info::is_global(const ast::var_decl* declaration) const noexcept
	{
		assert(declaration);
		return std::binary_search(
				std::begin(_globals), std::end(_globals), declaration,
				sem::detail::mem_comparator<ast::var_decl>{}
		);
	}

}  // namespace minijava
