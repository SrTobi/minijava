#include "firm/mangle.hpp"

#include <cassert>
#include <regex>


namespace minijava
{

	namespace /* anonymous */
	{

		// Declared `inline` to avoid `-Wunused-function` in release mode.
		inline bool is_valid_asm_id(const symbol id)
		{
			static const auto pattern = std::regex{"[A-Za-z_.][0-9A-Za-z_.]*"};
			return std::regex_match(id.c_str(), pattern);
		}

	}  // namespace /* anonymous */


	namespace firm
	{

		ident* mangle(firm_global_state& /* firm */,
					  const ast::class_declaration& clazz)
		{
			assert(is_valid_asm_id(clazz.name()));
			return new_id_fmt(
#ifdef _WIN32
				"Mj_%s_c%Iu",
#else
				"Mj_%s_c%zu",
#endif
				clazz.name().c_str(), clazz.name().length()
			);
		}

		ident* mangle(firm_global_state& /* firm */,
					  const ast::class_declaration& clazz,
					  const ast::var_decl& field)
		{
			assert(is_valid_asm_id(clazz.name()));
			assert(is_valid_asm_id(field.name()));
			assert(clazz.get_field(field.name()) == &field);
			return new_id_fmt(
#ifdef _WIN32
				"Mj_%s_c%Iu_%s_f%Iu",
#else
				"Mj_%s_c%zu_%s_f%zu",
#endif
				clazz.name().c_str(), clazz.name().length(),
				field.name().c_str(), field.name().length()
			);
		}

		ident* mangle(firm_global_state& /* firm */,
					  const ast::class_declaration& clazz,
					  const ast::instance_method& method)
		{
			assert(is_valid_asm_id(clazz.name()));
			assert(is_valid_asm_id(method.name()));
			assert(clazz.get_instance_method(method.name()) == &method);
			return new_id_fmt(
#ifdef _WIN32
				"Mj_%s_c%Iu_%s_m%Iu",
#else
				"Mj_%s_c%zu_%s_m%zu",
#endif
				clazz.name().c_str(),  clazz.name().length(),
				method.name().c_str(), method.name().length()
			);
		}

	}  // namespace firm

}  // namespace minijava
