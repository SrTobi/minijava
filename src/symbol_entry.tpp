#ifndef MINIJAVA_INCLUDED_FROM_SYMBOL_ENTRY_HPP
#error "Never `#include <symbol_entry.tpp>` directly; `#include <symbol_entry.hpp>` instead."
#endif

#include <algorithm>
#include <cassert>
#include <cstring>


namespace minijava
{

	namespace detail
	{

		constexpr std::size_t symbol_entry_allocation_size(std::size_t length) noexcept
		{
			constexpr auto sz = sizeof(symbol_entry);
			return 1 + (length + sz - 1) / sz;
		}

	}  // namespace detail


	template <typename AllocT>
	unique_symbol_entr_ptr<AllocT> new_symbol_entry(AllocT& alloc,
	                                                const std::size_t hash,
	                                                const std::size_t size,
	                                                const char *const data)
	{
		static_assert(std::is_same<symbol_entry, typename std::allocator_traits<AllocT>::value_type>{},
		              "symbol_entry_deleter<AllocT> can only work when AllocT::value_type is symbol_entry");
		assert(size > 0);
		assert(data != nullptr);
		using traits_type = std::allocator_traits<AllocT>;
		using deleter_type = symbol_entry_deleter<AllocT>;
		using raii_type = std::unique_ptr<symbol_entry, deleter_type>;
		const auto count = detail::symbol_entry_allocation_size(size);
		auto del = deleter_type{alloc};
		auto p = raii_type{traits_type::allocate(alloc, count), std::move(del)};
		p->hash = hash;
		p->size = size;
		std::memcpy(p->data, data, size);
		p->data[size] = '\0';
		return std::move(p); // explicit move to work around clang bug
	}

	template <typename AllocT>
	symbol_entry_deleter<AllocT>::symbol_entry_deleter(const AllocT& alloc)
		: AllocT{alloc}
	{
	}

	template <typename AllocT>
	void symbol_entry_deleter<AllocT>::operator()(const symbol_entry* p)
	{
		using traits_type = std::allocator_traits<allocator_type>;
		const auto memory = const_cast<symbol_entry*>(p);
		const auto size = detail::symbol_entry_allocation_size(p->size);
		traits_type::deallocate(*this, memory, size);
	}

	inline const symbol_entry * get_empty_symbol_entry() noexcept
	{
		static const auto entry = symbol_entry{0, 0, ""};
		return &entry;
	}

	template<typename SmartPtrT>
	std::size_t symbol_entry_ptr_hash::operator()(const SmartPtrT& entry) const noexcept
	{
		return entry->hash;
	}

	template <typename SmartPtrT>
	bool symbol_entry_ptr_equal::operator()(const SmartPtrT& lhs, const SmartPtrT& rhs) const noexcept
	{
		return std::equal(lhs->data, lhs->data + lhs->size,
		                  rhs->data, rhs->data + rhs->size);
	}

}  // namespace minijava
