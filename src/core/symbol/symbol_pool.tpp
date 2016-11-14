#ifndef MINIJAVA_INCLUDED_FROM_SYMBOL_SYMBOL_POOL_HPP
#error "Never `#include <symbol/symbol_pool.tpp>` directly; `#include <symbol/symbol_pool.hpp>` instead."
#endif

#include <algorithm>
#include <tuple>


namespace minijava
{

	namespace detail
	{

		struct symbol_entry_string_cmp
		{

			template <typename AllocT>
			bool operator()(const std::string& str,
			                const unique_symbol_entr_ptr<AllocT>& entry) const
			{
				return std::equal(str.begin(), str.end(),
				                  entry->data, entry->data + entry->size);
			}

		};

	}  // namespace detail


	template <typename AllocT>
	symbol_pool<AllocT>::symbol_pool()
		: _anchor{symbol_anchor::make_symbol_anchor()}
	{
	}

	template <typename AllocT>
	symbol_pool<AllocT>::symbol_pool(const allocator_type& alloc)
		: AllocT{alloc}
		, _anchor{symbol_anchor::make_symbol_anchor()}
	{
	}

	template <typename AllocT>
	symbol_pool<AllocT>::symbol_pool(symbol_pool&& other) noexcept
		: AllocT(other.get_allocator())
	{
		swap(*this, other);
	}

	template<typename AllocT>
	symbol_pool<AllocT>& symbol_pool<AllocT>::operator=(symbol_pool&& other) noexcept
	{
		symbol_pool temp{get_allocator()};
		swap(*this, temp);
		swap(*this, other);
		return *this;
	}

	template <typename AllocT>
	symbol symbol_pool<AllocT>::normalize(const std::string& text)
	{
		if (text.empty()) {
			return symbol{};
		}

		const auto hash_fn = std::hash<std::string>{};
		const auto comp_fn = detail::symbol_entry_string_cmp{};
		const auto hash = hash_fn(text);

		auto entry_it = _pool.find(text, hash_fn, comp_fn);

		if (entry_it == _pool.end()) {
			auto insert_entry = new_symbol_entry(get_allocator(), hash, text.size(), text.data());
			std::tie(entry_it, std::ignore) = _pool.insert(std::move(insert_entry));
		}

		return symbol{entry_it->get(), _anchor};
	}

	template <typename AllocT>
	bool symbol_pool<AllocT>::is_normalized(const std::string& text) const
	{
		if (text.empty()) {
			return true;
		}
		const auto hash_fn = std::hash<std::string>{};
		const auto comp_fn = detail::symbol_entry_string_cmp{};
		return (_pool.find(text, hash_fn, comp_fn) != _pool.cend());
	}

	template <typename AllocT>
	std::size_t symbol_pool<AllocT>::size() const noexcept
	{
		return _pool.size();
	}

	template <typename AllocT>
	bool symbol_pool<AllocT>::empty() const noexcept
	{
		return _pool.empty();
	}

	template <typename AllocT>
	const AllocT& symbol_pool<AllocT>::get_allocator() const noexcept
	{
		return *this;
	}

	template <typename AllocT>
	AllocT& symbol_pool<AllocT>::get_allocator() noexcept
	{
		return *this;
	}

}  // namespace minijava
