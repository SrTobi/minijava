#include "semantic/name_type_analysis.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>

#include <boost/optional.hpp>

#include "exceptions.hpp"
#include "semantic/semantic_error.hpp"
#include "symbol/symbol.hpp"

using namespace std::string_literals;


namespace minijava
{

	namespace sem
	{

		namespace /* anonymous */
		{

			basic_type_info primitive_type_info(ast::primitive_type primitive) {
				switch (primitive) {
				case ast::primitive_type::type_void:
					return basic_type_info::make_void_type();
				case ast::primitive_type::type_int:
					return basic_type_info::make_int_type();
				case ast::primitive_type::type_boolean:
					return basic_type_info::make_boolean_type();
				}
				MINIJAVA_NOT_REACHED();
			}

			type get_type(const ast::type& declared_type,
						  const class_definitions& classes,
						  const bool voidok)
			{
				const auto& declared_basic_type = declared_type.name();
				const auto rank = declared_type.rank();
				if (const auto primitive = boost::get<ast::primitive_type>(&declared_basic_type)) {
					const auto pti = primitive_type_info(*primitive);
					if (pti.is_void()) {
						if (rank > 0) {
							throw semantic_error{"Cannot have array of 'void'"};
						}
						if (!voidok) {
							throw semantic_error{"Variable cannot be 'void'"};
						}
					}
					return type{pti, rank};
				}
				const auto type_name = boost::get<symbol>(declared_basic_type);
				const auto pos = classes.find(type_name);
				if (pos == classes.end()) {
					throw semantic_error{"Unknown type '"s + type_name.c_str() + "'"};
				}
				return type{pos->second, rank};
			}

			void annotate_field(const ast::var_decl& field,
								const class_definitions& classes,
								type_attributes& type_annotations)
			{
				const auto thetype = get_type(field.var_type(), classes, false);
				type_annotations.put(field, thetype);
			}

			void annotate_method(const ast::method& method,
								 const class_definitions& classes,
								 type_attributes& type_annotations)
			{
				const auto thetype = get_type(method.return_type(), classes, true);
				type_annotations.put(method, thetype);
				for (const auto& param : method.parameters()) {
					const auto thetype = get_type(param->var_type(), classes, false);
					type_annotations.put(*param, thetype);
				}
			}

			[[noreturn]] void
			throw_duplicate_field(const ast::var_decl* f1,
								  const ast::var_decl* f2)
			{
				const ast::var_decl* first = f1;
				const ast::var_decl* second = f2;
				if (f1->line() > f2->line() || (f1->line() == f2->line()
						&& f1->column() > f2->column())) {
					std::swap(f1, f2);
				}
				auto oss = std::ostringstream{};
				oss << "Declaration of field '" << first->name() << "'"
					<< " on line " << first->line()
					<< " conflicts with previous declaration on line "
					<< second->line() << ".";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_duplicate_method(const ast::instance_method* m1,
								   const ast::instance_method* m2)
			{
				const ast::instance_method* first = m1;
				const ast::instance_method* second = m2;
				if (m1->line() > m2->line() || (m1->line() == m2->line()
						&& m1->column() > m2->column())) {
					std::swap(m1, m2);
				}
				auto oss = std::ostringstream{};
				oss << "Declaration of field '" << first->name() << "'"
					<< " on line " << first->line()
					<< " conflicts with previous declaration on line "
					<< second->line() << ".";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_another_main(const ast::class_declaration* c1,
							   const ast::class_declaration* c2)
			{
				const ast::class_declaration* first = c1;
				const ast::class_declaration* second = c2;
				if (c1->line() > c2->line() || (c1->line() == c2->line()
						&& c1->column() > c2->column())) {
					std::swap(c1, c2);
				}
				auto oss = std::ostringstream{};
				oss << "Class '" << second->name() << "'"
					<< " cannot declare another method 'public static main'"
					<< " because 'main' was already declared by class"
					<< " '" << first->name() << "'"
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

		}  // namespace /* anonymous */


		void perform_shallow_type_analysis(const ast::program& program,
										   const class_definitions& classes,
										   type_attributes& type_annotations)
		{
			const ast::class_declaration* main_class_ptr = nullptr;
			for (const auto& clazz : program.classes()) {
				auto last_name = symbol{};
				const ast::var_decl* last_field = nullptr;
				for (const auto& field : clazz->fields()) {
					if (field->name() == last_name) {
						throw_duplicate_field(field.get(), last_field);
					}
					last_field = field.get();
					last_name = field->name();
					annotate_field(*field, classes, type_annotations);
				}
				last_name = symbol{};
				const ast::instance_method* last_method = nullptr;
				for (const auto& method : clazz->instance_methods()) {
					if (method->name() == last_name) {
						throw_duplicate_method(method.get(), last_method);
					}
					last_method = method.get();
					last_name = method->name();
					annotate_method(*method, classes, type_annotations);
				}
				for (const auto& main : clazz->main_methods()) {
					annotate_method(*main, classes, type_annotations);
					if (main->name() != "main") {
						throw semantic_error{"Only 'main' can be 'static'"};
					}
					if (main_class_ptr != nullptr) {
						throw_another_main(clazz.get(), main_class_ptr);
					}
					main_class_ptr = clazz.get();
				}
			}
			if (main_class_ptr == nullptr) {
				throw_no_main();
			}
		}

		namespace /* anonymous */
		{

			class lvalue_visitor final : public ast::visitor
			{

			public:

				lvalue_visitor(const type_attributes& types)
						: _types{types} {}

				bool is_lvalue{false};

				using ast::visitor::visit;

				void visit(const ast::array_access& node) override
				{
					auto& type_info = _types.at(node).info;
					assert (!type_info.is_void());
					is_lvalue = type_info.is_user_defined() || type_info.is_primitive();
				}

				void visit(const ast::variable_access& node) override
				{
					auto& type_info = _types.at(node).info;
					assert (!type_info.is_void());
					is_lvalue = type_info.is_user_defined() || type_info.is_primitive();
				}

			private:

				const type_attributes& _types;

			};

		}  // namespace /* anonymous */

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

	}  // namespace sem

}  // namespace minijava
