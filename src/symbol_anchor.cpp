#include "symbol_anchor.hpp"

#include "global.hpp"


namespace minijava
{

	const std::weak_ptr<symbol_anchor>& symbol_anchor::get_empty_symbol_anchor() noexcept
	{
		static const auto shared_anchor = make_symbol_anchor();
		static const auto weak_anchor = std::weak_ptr<symbol_anchor>{shared_anchor};
		return weak_anchor;
	}

	std::shared_ptr<symbol_anchor> symbol_anchor::make_symbol_anchor()
	{
		if (MINIJAVA_ASSERT_ACTIVE) {
			// This might throw std::bad_alloc.
			return std::make_shared<symbol_anchor>();
		} else {
			// This doesn't allocate memory and will never throw.
			return std::shared_ptr<symbol_anchor>{};
		}
	}

}  // namespace minijava
