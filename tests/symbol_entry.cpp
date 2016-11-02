#include "symbol_entry.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <iostream>

#define BOOST_TEST_MODULE  symbol_entry
#include <boost/test/unit_test.hpp>


namespace /* anonymous */
{

	template <class T>
	struct mock_allocator
	{
		using  value_type = T;

		using journal_type = std::vector<std::pair<T *, std::size_t>>;

		// This type is not default-constructible!
		mock_allocator() = delete;

		mock_allocator(journal_type& alloc, journal_type& dealloc)
			: alloc_log{&alloc}, dealloc_log{&dealloc}
		{
		}

		template <class U>
		mock_allocator(const mock_allocator<U>& other)
			: alloc_log{other.alloc_log}, dealloc_log{other.dealloc_log}
		{
		}

		T * allocate(const std::size_t n)
		{
			const auto p = static_cast<T*>(std::malloc(n * sizeof(T)));
			if (alloc_log != nullptr) {
				alloc_log->push_back(std::make_pair(p, n));
			}
			std::clog << "\033[32m" << __func__ << "(" << n << ") = " << static_cast<void*>(p) << "\033[m" << std::endl;
			return p;
		}

		void deallocate(T *const p, const std::size_t n)
		{
			std::clog << "\033[31m" << __func__ << "(" << static_cast<void*>(p) << ", " << n << ")" << "\033[m" << std::endl;
			if (dealloc_log != nullptr) {
				dealloc_log->push_back(std::make_pair(p, n));
			}
			std::free(p);
		}

		journal_type * alloc_log{};
		journal_type * dealloc_log{};

	};

	template <class T, class U>
	bool operator==(const mock_allocator<T>& lhs, const mock_allocator<U>& rhs)
	{
		return (lhs.alloc_log == rhs.alloc_log) && (lhs.dealloc_log == rhs.dealloc_log);
	}

	template <class T, class U>
	bool operator!=(const mock_allocator<T>& lhs, const mock_allocator<U>& rhs)
	{
		return !(lhs == rhs);
	}

}  // namespace /* anonymous */


BOOST_AUTO_TEST_CASE(empty_symbol_entry_has_size_zero)
{
	const auto ep = minijava::get_empty_symbol_entry();
	BOOST_REQUIRE_EQUAL(std::size_t{0}, ep->size);
}


BOOST_AUTO_TEST_CASE(empty_symbol_entry_has_hash_zero)
{
	const auto ep = minijava::get_empty_symbol_entry();
	BOOST_REQUIRE_EQUAL(std::size_t{0}, ep->hash);
}


BOOST_AUTO_TEST_CASE(empty_symbol_entry_has_empty_string)
{
	const auto ep = minijava::get_empty_symbol_entry();
	BOOST_REQUIRE_EQUAL(std::size_t{0}, std::strlen(ep->data));
}


BOOST_AUTO_TEST_CASE(empty_symbol_entry_is_singleton)
{
	const auto cv = [](auto p){ return static_cast<const void*>(p); };
	const auto ep1 = minijava::get_empty_symbol_entry();
	const auto ep2 = minijava::get_empty_symbol_entry();
	BOOST_REQUIRE_EQUAL(cv(ep1), cv(ep2));
}


BOOST_AUTO_TEST_CASE(new_symbol_entry_initializes_object_correctly)
{
	using namespace std::string_literals;
	const auto text = "The quick brown fox jumps over the sleazy dog."s;
	const auto hash = std::size_t{42};
	auto alloc = std::allocator<minijava::symbol_entry>{};
	const auto sep = minijava::new_symbol_entry(alloc, hash, text.size(), text.data());
	BOOST_REQUIRE_EQUAL(hash, sep->hash);
	BOOST_REQUIRE_EQUAL(text.size(), sep->size);
	BOOST_REQUIRE_EQUAL('\0', sep->data[text.size()]);
	BOOST_REQUIRE_EQUAL(text, sep->data);
}


BOOST_AUTO_TEST_CASE(new_symbol_entry_uses_allocator_correctly)
{
	using namespace std::string_literals;
	const auto text = "The quick brown fox jumps over the sleazy dog."s;
	const auto hash = std::size_t{42};
	auto alloc_log = mock_allocator<minijava::symbol_entry>::journal_type{};
	auto dealloc_log = mock_allocator<minijava::symbol_entry>::journal_type{};
	auto alloc = mock_allocator<minijava::symbol_entry>{alloc_log, dealloc_log};
	{
		const auto sep = minijava::new_symbol_entry(alloc, hash, text.size(), text.data());
		BOOST_REQUIRE_EQUAL(std::size_t{1}, alloc_log.size());
		BOOST_REQUIRE_EQUAL(std::size_t{0}, dealloc_log.size());
	}
	BOOST_REQUIRE_EQUAL(std::size_t{1}, alloc_log.size());
	BOOST_REQUIRE_EQUAL(std::size_t{1}, dealloc_log.size());
	BOOST_REQUIRE(alloc_log.front() == dealloc_log.front());
	constexpr auto sz = sizeof(minijava::symbol_entry);
	BOOST_REQUIRE_LE(sz + text.size(), sz * alloc_log.front().second);
}
