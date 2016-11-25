#ifndef MINIJAVA_INCLUDED_FROM_SEMANTIC_SYMBOL_TABLE_HPP
#error "Never `#include <semantic/symbol_table.tpp>` directly; `#include <semantic/symbol_table.hpp>` instead."
#endif

#include <cassert>

namespace minijava
{

	namespace sem
	{

		namespace detail
		{

			// Unconditionally `throw`s a `semantic_error` complaining about a
			// redefinition of symbol `name` at position `curr` conflicting
			// with previous definition at position `prev`.
			[[noreturn]] void throw_conflicting_symbol_definitions(
				symbol name, const ast::var_decl* curr, const ast::var_decl* prev
			);

		}

		inline symbol_table::symbol_table() noexcept
		{
		}

		inline const ast::var_decl*
		symbol_table::lookup(const symbol name) const
		{
			assert(!_nested_scopes.empty());
			const auto first = std::rbegin(_nested_scopes);
			const auto last = std::rend(_nested_scopes);
			for (auto it = first; it != last; ++it) {
				const auto pos = it->defs.find(name);
				if (pos != it->defs.end()) {
					return pos->second;
				}
			}
			return nullptr;
		}

		inline const ast::var_decl*
		symbol_table::get_conflicting_definitions(const symbol name) const
		{
			assert(!_nested_scopes.empty());
			const auto first = std::rbegin(_nested_scopes);
			const auto last = std::rend(_nested_scopes);
			for (auto it = first; it != last; ++it) {
				if ((it == first) || !it->may_shadow) {
					const auto pos = it->defs.find(name);
					if (pos != it->defs.end()) {
						return pos->second;
					}
				}
			}
			return nullptr;
		}

		inline void symbol_table::add_def(const ast::var_decl* def)
		{
			assert(!_nested_scopes.empty());
			assert(def);
			if (const auto xxx = get_conflicting_definitions(def->name())) {
				detail::throw_conflicting_symbol_definitions(def->name(), def, xxx);
			}
			_nested_scopes.back().defs[def->name()] = def;
		}

		inline void symbol_table::enter_scope(const bool may_shadow)
		{
			_nested_scopes.emplace_back();
			_nested_scopes.back().may_shadow = may_shadow;
		}

		inline void symbol_table::leave_scope()
		{
			assert(!_nested_scopes.empty());
			_nested_scopes.pop_back();
		}

		inline std::size_t symbol_table::depth() const noexcept
		{
			return _nested_scopes.size();
		}

	}  // namespace sem

}  // namespace minijava
