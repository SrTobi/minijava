#include "semantic/type_info.hpp"

#include <sstream>
#include <string>
#include <utility>

#include "semantic/semantic_error.hpp"

namespace minijava
{

	namespace sem
	{

		namespace /* anonymous */
		{
			[[noreturn]] void
			throw_class_name_clash(const ast::class_declaration* c1,
								   const ast::class_declaration* c2)
			{
				const ast::class_declaration* first = c1;
				const ast::class_declaration* second = c2;
				if (c1->line() > c2->line() || (c1->line() == c2->line()
						&& c1->column() > c2->column())) {
					std::swap(c1, c2);
				}
				auto oss = std::ostringstream{};
				oss << "Declaration of class '" << first->name() << "'"
					<< " on line " << first->line()
					<< " conflicts with previous declaration on line "
					<< second->line() << ".";
				throw semantic_error{oss.str()};
			}
		}  // namespace /* anonymous */


		void extract_type_info(const ast::program& ast, bool builtin,
							   class_definitions& definitions)
		{
			using namespace std::string_literals;
			const ast::class_declaration* last_class = nullptr;
			for (const auto& clazz : ast.classes()) {
				if (definitions.find(clazz->name()) != definitions.end()) {
					throw_class_name_clash(clazz.get(), last_class);
				}
				last_class = clazz.get();
				definitions.insert(std::make_pair(
						clazz->name(),
						basic_type_info{*clazz, builtin}
				));
			}
		}
	}
}
