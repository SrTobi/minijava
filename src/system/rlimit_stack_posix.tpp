#ifndef MINIJAVA_INCLUDED_FROM_SYSTEM_SYSTEM_HPP
#error "Never `#include` the source file `<system/rlimit_stack_posix.tpp>`"
#endif

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <system_error>

#include <sys/resource.h>


namespace minijava
{

	namespace /* anonymous */
	{

		// Safely casts a non-negative integer without triggering overflow.
		//
		// If `DstT` cannot hold `val`, `inf` is `return`ed.  Otherwise, if
		// `inf` is non-negative and the value of `val` is greater than or
		// equal to `inf`, `inf` is `return`ed.  Otherwise, `val` casted to
		// `DstT` is `return`ed.
		//
		// `DstT` and `SrcT` may be `signed` or `unsigned` integer types and
		// `inf` may be negative.  The behavior is undefined if `val` is
		// negative.
		template <typename DstT, typename SrcT>
		constexpr DstT cast_or_infinity_non_negative(const SrcT src_val, const DstT dst_inf)
		{
			const auto src_0 = SrcT{0}; (void) src_0;  // Dear GCC, just because you fold
			const auto dst_0 = DstT{0}; (void) dst_0;  // these constants as you ought to
			assert(src_val >= src_0);                  // doesn't mean I'm not using them.
			const auto dst_max = std::numeric_limits<DstT>::max();
			const auto xxl_max = static_cast<std::uintmax_t>(dst_max);
			const auto xxl_val = static_cast<std::uintmax_t>(src_val);
			if (xxl_val > xxl_max) {
				return dst_inf;
			}
			const auto dst_val = static_cast<DstT>(src_val);
			const auto gt_inf_eh = ((dst_inf >= dst_0) && (dst_val > dst_inf));
			const auto dst_res = gt_inf_eh ? dst_inf : dst_val;
			assert(
				(dst_inf >= dst_0)
					? ((dst_res >= dst_0) && (dst_res <= dst_inf))
					: ((dst_res >= dst_0) || (dst_res == dst_inf))
			);
			return dst_res;
		}

		// Since writing the above function costed me the better part of the
		// overall time required to implement this feature, let's test it a
		// little to make sure I got it right.

		namespace cast_or_infinity_non_negative_sef_tests
		{

			// basic cases

			static_assert(0 == cast_or_infinity_non_negative<int>(0, 0), "");
			static_assert(0 == cast_or_infinity_non_negative<int>(0, -1), "");
			static_assert(10 == cast_or_infinity_non_negative<int>(10, -1), "");
			static_assert(5 == cast_or_infinity_non_negative<int>(10, 5), "");

			// overflow corner cases

			using st = std::intmax_t;
			constexpr auto s0 = st{0};
			constexpr auto s1 = st{1};
			constexpr auto sx = std::numeric_limits<st>::max();

			using ut = std::uintmax_t;
			constexpr auto u0 = ut{0};
			constexpr auto u1 = ut{1};
			constexpr auto ux = std::numeric_limits<ut>::max();

			constexpr auto usx = static_cast<ut>(sx);

			static_assert(sx == cast_or_infinity_non_negative<st>(ux, sx), "");
			static_assert(st{-17} == cast_or_infinity_non_negative<st>(ux, st{-17}), "");
			static_assert(s1 == cast_or_infinity_non_negative<st>(ux, s1), "");
			static_assert(s0 == cast_or_infinity_non_negative<st>(ux, s0), "");
			static_assert(usx == cast_or_infinity_non_negative<ut>(sx, ux), "");

		}  // namespace cast_or_infinity_non_negative_sef_tests

	}  // namespace /* anonymous */


	std::ptrdiff_t set_max_stack_size_limit(const std::ptrdiff_t limit)
	{
		auto rl = rlimit{};
		if (getrlimit(RLIMIT_STACK, &rl) < 0) {
			throw std::system_error{errno, std::system_category()};
		}
		const auto previous = rl.rlim_cur;
		if (limit != std::ptrdiff_t{0}) {
			rl.rlim_cur = (limit < std::ptrdiff_t{0})
				? rl.rlim_max
				: cast_or_infinity_non_negative<rlim_t>(limit, rl.rlim_max);
			if (setrlimit(RLIMIT_STACK, &rl) < 0) {
				throw std::system_error{errno, std::system_category()};
			}
		}
		return cast_or_infinity_non_negative<std::ptrdiff_t>(previous, std::ptrdiff_t{-1});
	}


}  // namespace minijava
