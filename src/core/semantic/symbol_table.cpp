#include "semantic/symbol_table.hpp"

#include <sstream>

#include "semantic/semantic_error.hpp"


namespace minijava
{

	namespace sem
	{

		namespace detail
		{

			[[noreturn]] void throw_conflicting_symbol_definitions(
				const symbol name,
				const ast::var_decl*const curr,
				const ast::var_decl*const /* prev */
			)
			{
				// TODO: Set position attribute on exception object once it has it.
				// const auto curr_pos = (curr != nullptr) ? curr->position() : position{};
				// const auto prev_pos = (prev != nullptr) ? prev->position() : position{};
				auto oss = std::ostringstream{};
				oss << "Redefinition of symbol '" << name << "' "
					<< "conflicts with previous declaration";
				throw semantic_error{oss.str(), curr->position()};
			}

		}  // namespace detail

	}  // namespace sem

}  // namespace minijava
