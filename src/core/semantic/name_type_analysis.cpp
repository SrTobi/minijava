#include "semantic/name_type_analysis.hpp"

#include <string>

#include "exceptions.hpp"
#include "semantic/semantic_error.hpp"
#include "symbol/symbol.hpp"


namespace minijava
{

	namespace sem
	{

		void perform_shallow_type_analysis(const ast::program& program,
										   const class_definitions& classes,
										   type_attributes& types)
		{
			using namespace std::string_literals;
			bool found_main{false};
			for (const auto& clazz : program.classes()) {
				auto last_name = symbol{};
				for (const auto& field : clazz->fields()) {
					if (field->name() == last_name) {
						throw semantic_error{
								"Duplicate field name: "s + field->name().c_str()
						};
					}
					// FIXME
					(void) classes;
					(void) types;
					//auto type_name = field->var_type().name();
					//auto type = types.find(field->var_type().name());
					//if ()
				}
				last_name = symbol{};
				for (const auto& method : clazz->instance_methods()) {
					if (method->name() == last_name) {
						throw semantic_error{
								"Duplicate method name: "s + method->name().c_str()
						};
					}
					// FIXME
				}
				for (const auto& main : clazz->main_methods()) {
					if (found_main) {
						throw semantic_error{
								"Found more than one main method."
						};
					}
					found_main = true;
					if (main->name() != "name") {
						throw semantic_error{
								"Main method has incorrect name: "s + main->name().c_str()
						};
					}
				}
			}
			if (!found_main) {
				throw semantic_error{"Found no main method."};
			}
		}

		void perform_full_name_type_analysis(const ast::program&      ast,
											 const class_definitions& classes,
											 const globals_map&       /* globals             */,
											 type_attributes&         type_annotations,
											 locals_attributes&       /* locals_annotations  */,
											 vardecl_attributes&      /* vardecl_annotations */,
											 method_attributes&       /* method_annotations  */)
		{
			perform_shallow_type_analysis(ast, classes, type_annotations);
			MINIJAVA_THROW_ICE(not_implemented_error);
		}
	}
}
