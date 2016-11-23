#include "semantic/type_info.hpp"

#include <string>

#include "semantic/semantic_error.hpp"

namespace minijava
{
	namespace sem
	{
		void extract_type_info(const ast::program& ast, bool builtin,
							   type_definitions& definitions)
		{
			using namespace std::string_literals;
			for (const auto& clazz : ast.classes()) {
				if (definitions.find(clazz->name()) != definitions.end()) {
					throw semantic_error{"Duplicate class name: "s + clazz->name().c_str()};
				}
				definitions.insert(std::make_pair(
						clazz->name(),
						basic_type_info{*clazz, builtin}
				));
			}
		}
	}
}
