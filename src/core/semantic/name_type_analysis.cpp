#include "semantic/name_type_analysis.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>
#include <utility>

#include <boost/optional.hpp>

#include "exceptions.hpp"
#include "semantic/semantic_error.hpp"
#include "symbol/symbol.hpp"


namespace minijava
{

	namespace sem
	{

		namespace /* anonymous */
		{

			template <typename ContainerT>
			auto get_first_name_clash(ContainerT&& nodes)
			{
				using pointer_type = decltype(nodes.front().get());
				auto clash = boost::optional<std::pair<pointer_type, pointer_type>>{};
				const auto samename = [](auto&& lhs, auto&& rhs){
					return (lhs->name() == rhs->name());
				};
				const auto first = std::begin(nodes);
				const auto last = std::end(nodes);
				const auto pos = std::adjacent_find(first, last, samename);
			    if (pos != last) {
					clash = std::make_pair(pos->get(), std::next(pos)->get());
				}
				return clash;
			}

			void
			register_method(const ast::method& method,
							type_attributes& type_annotations)
			{
				assert(!type_annotations.count(&method));
			}

			void
			register_field(const ast::var_decl& field,
						   type_attributes& type_annotations)
			{
				assert(!type_annotations.count(&field));
			}

			[[noreturn]] void
			throw_another_main(const ast::class_declaration& has_another_main,
							   const ast::class_declaration& alredy_had_a_main)
			{
				auto oss = std::ostringstream{};
				oss << "Class '" << has_another_main.name() << "'"
					<< " cannot declare another method 'public static main'"
					<< " because 'main' was already declared by class"
					<< " '" << alredy_had_a_main.name() << "'"
					<< " and there can only be a single 'main' in a program";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void throw_no_main()
			{
				auto oss = std::ostringstream{};
				oss << "No program entry point found;"
					<< " please declare a single method with signature"
					<< " 'public static void main(String[] args)'";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_class_name_clash(const std::pair<const ast::class_declaration*, const ast::class_declaration*> clash)
			{
				auto oss = std::ostringstream{};
				oss << "Declaration of class '" << clash.first->name() << "'"
					<< " conflicts with previous declaration";
				throw semantic_error{oss.str()};
			}

		}  // namespace /* anonymous */


		void perform_shallow_type_analysis(const ast::program& program,
										   const class_definitions& /* classes */,
										   type_attributes& type_annotations)
		{
			const ast::class_declaration* main_class_ptr = nullptr;
			if (const auto clash = get_first_name_clash(program.classes())) {
				throw_class_name_clash(clash.get());
			}
			for (const auto& clazz : program.classes()) {
				for (const auto& field : clazz->fields()) {
					register_field(*field, type_annotations);
				}
				for (const auto& method : clazz->instance_methods()) {
					register_method(*method, type_annotations);
				}
				for (const auto& method : clazz->main_methods()) {
					register_method(*method, type_annotations);
					if (method->name() != "main") {
						throw semantic_error{"Only 'main' can be 'static'"};
					}
					if (main_class_ptr != nullptr) {
						throw_another_main(*clazz, *main_class_ptr);
					}
					main_class_ptr = clazz.get();
				}
			}
			if (main_class_ptr == nullptr) {
				throw_no_main();
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
