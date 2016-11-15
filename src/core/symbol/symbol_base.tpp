#ifndef MINIJAVA_INCLUDED_FROM_SYMBOL_SYMBOL_HPP
#error "Never `#include <symbol/symbol_base.tpp>` directly; `#include <symbol/symbol.hpp>` instead."
#endif


namespace minijava
{

	namespace detail
	{

		class symbol_debug_base
		{
		public:

			symbol_debug_base(const std::weak_ptr<symbol_anchor>& anchor)
				: _anchor(anchor)
			{
			}

			static bool have_compatible_pool(const symbol_debug_base& lhs,
			                                 const symbol_debug_base& rhs) noexcept
			{
		        static const auto ep = symbol_anchor::get_empty_symbol_anchor().lock().get();
				const auto lp = lhs._anchor.lock().get();
				const auto rp = rhs._anchor.lock().get();
				return (lp == ep) || (rp == ep) || (lp == rp);
			}

			bool is_pool_available() const noexcept
			{
				return !_anchor.expired();
			}

		private:

			std::weak_ptr<symbol_anchor> _anchor{};

		};

		class symbol_release_base
		{
		public:

			constexpr symbol_release_base(const std::weak_ptr<symbol_anchor>& /* anchor */) noexcept
			{
			}

			static constexpr bool have_compatible_pool(const symbol_release_base& /* lhs */,
			                                           const symbol_release_base& /* rhs */) noexcept
			{
				return true;
			}

			static constexpr bool is_pool_available() noexcept
			{
				return true;
			}

		};

		using symbol_base = std::conditional_t<
			MINIJAVA_ASSERT_ACTIVE,
			symbol_debug_base,
			symbol_release_base
		>;

	}  // namespace detail

}  // namespace minijava
