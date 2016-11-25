#include "semantic/name_type_analysis.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

#include <boost/optional.hpp>

#include "exceptions.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/symbol_table.hpp"
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
			throw_duplicate_main(const ast::class_declaration* c1,
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

			[[noreturn]] void throw_unexpected_main()
			{
				auto oss = std::ostringstream{};
				oss << "Program entry point found although none was expected.";
				throw semantic_error{oss.str()};
			}

		}  // namespace /* anonymous */


		void perform_shallow_type_analysis(const ast::program& program,
										   const class_definitions& classes,
										   type_attributes& type_annotations,
										   bool expect_main)
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
						throw_duplicate_main(clazz.get(), main_class_ptr);
					}
					main_class_ptr = clazz.get();
				}
			}
			if (main_class_ptr == nullptr) {
				if (expect_main) {
					throw_no_main();
				}
			} else if (!expect_main) {
				throw_unexpected_main();
			}
		}

		namespace /* anonymous */
		{

			[[noreturn]] void
			throw_return_void_expected(const ast::method& m,
									   const ast::return_statement& s)
			{
				(void) s;
				auto oss = std::ostringstream{};
				oss << "Method " << m.name().c_str() << " has return type void"
					<< " and therefore cannot return any value.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_return_value_expected(const ast::method& m,
										const ast::return_statement& s)
			{
				(void) s;
				auto oss = std::ostringstream{};
				oss << "Method " << m.name().c_str() << " has non-void return"
					<< " type and therefore must return a value.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_incompatible_type(type expected, type actual,
									const ast::expression& expr)
			{
				(void) expr;
				std::ostringstream oss{};
				oss << "Type of expression (" << actual
					<< ") is not compatible with the expected type ("
					<< expected << ").";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_unknown_type(symbol unknown_type, const ast::object_instantiation& expr)
			{
				(void) expr;
				std::ostringstream oss{};
				oss << "Unknown type '" << unknown_type.c_str() << "'.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_invalid_subscript(type actual, const ast::array_access& expr)
			{
				(void) expr;
				std::ostringstream oss{};
				oss << "Subscript operator used on non-array type '" << actual << "'.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_invalid_field_access(type actual, const ast::variable_access& expr)
			{
				(void) expr;
				std::ostringstream oss{};
				oss << "Tried to access field on non-object type '" << actual << "'.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_invalid_method_access(type actual, const ast::method_invocation& expr)
			{
				(void) expr;
				std::ostringstream oss{};
				oss << "Tried to access method on non-object type '" << actual << "'.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_unknown_field(const ast::class_declaration& clazz,
								const ast::variable_access& node)
			{
				std::ostringstream oss{};
				oss << "Tried to access field '" << node.name().c_str()
					<< "' on variable of type '" << clazz.name().c_str()
					<< "', which does not have such a field.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_unknown_local_or_field(const ast::variable_access& node)
			{
				std::ostringstream oss{};
				oss << "Unknown variable or field '" << node.name().c_str()
					<< "'.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_illegal_access(const ast::variable_access& node)
			{
				std::ostringstream oss{};
				oss << "Tried to access '" << node.name().c_str()
					<< "', which is illegal.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_unqualified_method_access_in_main(const ast::method_invocation& node)
			{
				std::ostringstream oss{};
				oss << "Tried to access method '" << node.name().c_str()
					<< "' without any qualifier in the main method.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_unknown_method(const ast::class_declaration& clazz,
								 const ast::method_invocation& node)
			{
				std::ostringstream oss{};
				oss << "Tried to access method '" << node.name().c_str()
					<< "' on variable of type '" << clazz.name().c_str()
					<< "', which does not have such a method.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_incorrect_argument_count(std::size_t expected_size,
										   std::size_t actual_size,
										   const ast::method_invocation& node)
			{
				std::ostringstream oss{};
				oss << "Tried to call method '" << node.name().c_str()
					<< "', which has " << expected_size
					<< " parameters, with " << actual_size << " arguments.";
				throw semantic_error{oss.str()};
			}

			[[noreturn]] void
			throw_lvalue_expected(const ast::expression& expr)
			{
				(void) expr;
				throw semantic_error{"Expression cannot be used on the left side of an assignment."};
			}

			class lvalue_visitor final : public ast::visitor
			{

			public:

				lvalue_visitor(const type_attributes& types)
						: _types{types} {}

				bool is_lvalue{false};

				using ast::visitor::visit;

				void visit(const ast::array_access& node) override
				{
					const auto type_info = _types.at(node).info;
					assert (!type_info.is_void());
					is_lvalue = type_info.is_user_defined() || type_info.is_primitive();
				}

				void visit(const ast::variable_access& node) override
				{
					const auto type_info = _types.at(node).info;
					assert (!type_info.is_void());
					is_lvalue = type_info.is_user_defined() || type_info.is_primitive();
				}

			private:

				const type_attributes& _types;

			};

			class name_type_visitor final : public ast::visitor
			{

			public:

				name_type_visitor(const class_definitions& classes,
								  const globals_vector&    globals,
								  type_attributes&         type_annotations,
								  locals_attributes&       locals_annotations,
								  vardecl_attributes&      vardecl_annotations,
								  method_attributes&       method_annotations)
						: _classes{classes}, _globals{globals},
						  _type_annotations{type_annotations},
						  _locals_annotations{locals_annotations},
						  _vardecl_annotations{vardecl_annotations},
						  _method_annotations{method_annotations}
				{}

			protected:

				void visit_method(const ast::method& node) override
				{
					_symbols.enter_scope(false);
					_cur_method = &node;
					// bracket operator creates the set automatically
					auto locals_set = _locals_annotations[node];
					for (const auto& param : node.parameters()) {
						_symbols.add_def(param.get());
						locals_set.insert(param.get());
					}
					visit(node.body());
					_cur_method = nullptr;
					_symbols.leave_scope();
				}

			public:

				using ast::visitor::visit;

				void visit(const ast::var_decl& node) override
				{
					auto type = get_type(node.var_type(), _classes, false);
					_symbols.add_def(&node);
					if (_cur_method) {
						auto locals_set = _locals_annotations[*_cur_method];
						locals_set.insert(&node);
					}
					_type_annotations.put(node, type);
				}

				void visit(const ast::binary_expression& node) override
				{
					const auto& lhs = node.lhs();
					const auto& rhs = node.rhs();
					lhs.accept(*this);
					rhs.accept(*this);
					auto lhs_type = _type_annotations.at(lhs);
					auto rhs_type = _type_annotations.at(rhs);
					switch (node.type()) {
					case ast::binary_operation_type::assign:
					{
						auto visitor = lvalue_visitor{_type_annotations};
						lhs.accept(visitor);
						if (!visitor.is_lvalue) {
							throw_lvalue_expected(lhs);
						}
						check_type(lhs_type, rhs);
						_type_annotations.put(node, lhs_type);
						break;
					}
					case ast::binary_operation_type::logical_or:
					case ast::binary_operation_type::logical_and:
						check_boolean(lhs);
						check_boolean(rhs);
						_type_annotations.put(
								node, {basic_type_info::make_boolean_type(), 0}
						);
						break;
					case ast::binary_operation_type::equal:
					case ast::binary_operation_type::not_equal:
						if (!is_assignable(lhs_type, rhs_type)
								&& !is_assignable(rhs_type, lhs_type)) {
							throw_incompatible_type(lhs_type, rhs_type, node);
						}
						_type_annotations.put(
								node, {basic_type_info::make_boolean_type(), 0}
						);
						break;
					case ast::binary_operation_type::less_than:
					case ast::binary_operation_type::less_equal:
					case ast::binary_operation_type::greater_than:
					case ast::binary_operation_type::greater_equal:
						check_integer(lhs);
						check_integer(rhs);
						_type_annotations.put(
								node, {basic_type_info::make_boolean_type(), 0}
						);
						break;
					case ast::binary_operation_type::plus:
					case ast::binary_operation_type::minus:
					case ast::binary_operation_type::multiply:
					case ast::binary_operation_type::divide:
					case ast::binary_operation_type::modulo:
						check_integer(lhs);
						check_integer(rhs);
						_type_annotations.put(
								node, {basic_type_info::make_int_type(), 0}
						);
						break;
					default:
						MINIJAVA_NOT_REACHED();
					}
				}

				void visit(const ast::unary_expression& node) override
				{
					const auto& target = node.target();
					target.accept(*this);
					switch (node.type()) {
					case ast::unary_operation_type::logical_not:
						check_boolean(target);
						_type_annotations.put(
								node, {basic_type_info::make_boolean_type(), 0}
						);
						break;
					case ast::unary_operation_type::minus:
						check_integer(target);
						_type_annotations.put(
								node, {basic_type_info::make_int_type(), 0}
						);
						break;
					default:
						MINIJAVA_NOT_REACHED();
					}
				}

				void visit(const ast::object_instantiation& node) override
				{
					auto type_name = node.class_name();
					auto type_it = _classes.find(type_name);
					if (type_it == _classes.end()) {
						throw_unknown_type(type_name, node);
					} else {
						_type_annotations.put(node, {type_it->second, 0});
					}
				}

				void visit(const ast::array_instantiation& node) override
				{
					auto type = get_type(node.array_type(), _classes, false);
					const auto& extent = node.extent();
					extent.accept(*this);
					check_integer(extent);
					_type_annotations.put(node, type);
				}

				void visit(const ast::array_access& node) override
				{
					const auto& target = node.target();
					const auto& index = node.index();
					target.accept(*this);
					auto target_type = _type_annotations.at(target);
					if (target_type.rank == 0) {
						throw_invalid_subscript(target_type, node);
					}
					index.accept(*this);
					check_integer(index);
					_type_annotations.put(
							node, {target_type.info, target_type.rank - 1}
					);
				}

				void visit(const ast::variable_access& node) override
				{
					const auto target = node.target();
					if (target) {
						target->accept(*this);
						auto target_type = _type_annotations.at(*target);
						if (target_type.rank > 0 || !target_type.info.is_reference()) {
							throw_invalid_field_access(target_type, node);
						}
						auto clazz = target_type.info.declaration();
						assert(clazz);
						auto range = clazz->find_fields(node.name());
						if (range.first == range.second) {
							throw_unknown_field(*clazz, node);
						}
						assert(range.first + 1 == range.second);
						auto decl = range.first->get();
						_vardecl_annotations.put(node, decl);
						_type_annotations.put(node, _type_annotations.at(*decl));
					} else {
						auto name = node.name();
						if (name == _poisoned_symbol) {
							throw_illegal_access(node);
						}
						auto decl = _symbols.lookup(name);
						if (!decl) {
							throw_unknown_local_or_field(node);
						}
						_vardecl_annotations.put(node, decl);
						_type_annotations.put(node, _type_annotations.at(*decl));
					}
				}

				void visit(const ast::method_invocation& node) override
				{
					const auto target = node.target();
					const ast::class_declaration* clazz{nullptr};
					if (target) {
						target->accept(*this);
						auto target_type = _type_annotations.at(*target);
						if (target_type.rank > 0 || !target_type.info.is_reference()) {
							throw_invalid_method_access(target_type, node);
						}
						clazz = target_type.info.declaration();
					} else {
						if (in_main()) {
							throw_unqualified_method_access_in_main(node);
						}
						clazz = _this_type.declaration();
					}
					assert(clazz);
					auto range = clazz->find_instance_methods(node.name());
					if (range.first == range.second) {
						throw_unknown_method(*clazz, node);
					}
					assert(range.first + 1 == range.second);
					auto decl = range.first->get();
					const auto& parameters = decl->parameters();
					const auto& arguments = node.arguments();
					const auto expected_size = parameters.size();
					const auto actual_size = arguments.size();
					if (expected_size != actual_size) {
						throw_incorrect_argument_count(expected_size, actual_size, node);
					}
					for (auto i = std::size_t{}; i < actual_size; ++i) {
						const auto& argument = *arguments[i];
						const auto& parameter = *parameters[i];
						argument.accept(*this);
						check_type(_type_annotations.at(parameter), argument);
					}
					_method_annotations.put(node, decl);
					_type_annotations.put(node, _type_annotations.at(*decl));
				}

				void visit(const ast::this_ref& node) override
				{
					if (in_main()) {
						throw semantic_error("Can not reference 'this' in static method!");
					}
					_type_annotations.put(
							node, {_this_type, 0}
					);
				}

				void visit(const ast::boolean_constant& node) override
				{
					_type_annotations.put(
							node, {basic_type_info::make_boolean_type(), 0}
					);
				}

				void visit(const ast::integer_constant& node) override
				{
					_type_annotations.put(
							node, {basic_type_info::make_int_type(), 0}
					);
				}

				void visit(const ast::null_constant& node) override
				{
					_type_annotations.put(
							node, {basic_type_info::make_null_type(), 0}
					);
				}

				void visit(const ast::local_variable_statement& node) override
				{
					const auto& decl = node.declaration();
					const auto initial_expr = node.initial_value();
					visit(decl);
					if (initial_expr) {
						initial_expr->accept(*this);
						check_type(_type_annotations.at(decl), *initial_expr);
					}
				}

				void visit(const ast::expression_statement& node) override
				{
					node.inner_expression().accept(*this);
				}

				void visit(const ast::block& node) override
				{
					_symbols.enter_scope(false);
					for (const auto& stmt : node.body()) {
						stmt->accept(*this);
					}
					_symbols.leave_scope();
				}

				void visit(const ast::if_statement& node) override
				{
					const auto& condition = node.condition();
					condition.accept(*this);
					check_boolean(condition);
					node.then_statement().accept(*this);
					if (node.else_statement())
						node.else_statement()->accept(*this);
				}

				void visit(const ast::while_statement& node) override
				{
					const auto& condition = node.condition();
					condition.accept(*this);
					check_boolean(condition);
					node.body().accept(*this);
				}

				void visit(const ast::return_statement& node) override
				{
					assert(_cur_method);
					auto return_type = _type_annotations.at(*_cur_method);
					const auto expr = node.value();
					if (return_type.info.is_void()) {
						if (expr) {
							throw_return_void_expected(*_cur_method, node);
						}
					} else {
						if (!expr) {
							throw_return_value_expected(*_cur_method, node);
						}
						expr->accept(*this);
						check_type(return_type, *expr);
					}
				}

				void visit(const ast::main_method& node) override
				{
					_poisoned_symbol = node.argname();
					visit_method(node);
					_poisoned_symbol = symbol{};
				}

				void visit(const ast::class_declaration& node) override
				{
					_symbols.enter_scope(true);
					_this_type = _classes.at(node.name());
					for (const auto& main : node.main_methods()) {
						visit(*main);
					}
					for (const auto& field : node.fields()) {
						_symbols.add_def(field.get());
					}
					for (const auto& method : node.instance_methods()) {
						visit(*method);
					}
					_this_type = basic_type_info::make_void_type();
					_symbols.leave_scope();
				}

				void visit(const ast::program& node) override
				{
					_symbols.enter_scope(true);
					for (const auto& glob : _globals) {
						visit(*glob);
					}
					for (const auto& clazz : node.classes()) {
						visit(*clazz);
					}
					_symbols.leave_scope();
				}

			private:

				const class_definitions& _classes;
				const globals_vector& _globals;
				type_attributes& _type_annotations;
				locals_attributes& _locals_annotations;
				vardecl_attributes& _vardecl_annotations;
				method_attributes& _method_annotations;

				symbol_table _symbols{};
				basic_type_info _this_type{basic_type_info::make_void_type()};
				const ast::method* _cur_method{nullptr};
				symbol _poisoned_symbol{};

				bool in_main() {
					return !_poisoned_symbol.empty();
				}

				static bool is_assignable(type target, type source) {
					return source == target || (source.info.is_null()
							&& target.rank == 0 && target.info.is_reference());
				}

				void check_boolean(const ast::expression& expr) {
					check_type({basic_type_info::make_boolean_type(), 0}, expr);
				}

				void check_integer(const ast::expression& expr) {
					check_type({basic_type_info::make_int_type(), 0}, expr);
				}

				void check_type(type expected, const ast::expression& expr) {
					assert(_type_annotations.find(&expr) != _type_annotations.end());
					auto expr_type = _type_annotations.at(expr);
					if (!is_assignable(expected, expr_type)) {
						throw_incompatible_type(expected, expr_type, expr);
					};
				}

			};

		}  // namespace /* anonymous */

		void perform_full_name_type_analysis(const ast::program&      ast,
											 const class_definitions& classes,
											 const globals_vector&    globals,
											 type_attributes&         type_annotations,
											 locals_attributes&       locals_annotations,
											 vardecl_attributes&      vardecl_annotations,
											 method_attributes&       method_annotations)
		{
			perform_shallow_type_analysis(ast, classes, type_annotations, true);
			auto visitor = name_type_visitor{
					classes, globals, type_annotations, locals_annotations,
					vardecl_annotations, method_annotations
			};
			ast.accept(visitor);
		}

		std::ostream& operator<<(std::ostream& os, const type typ)
		{
			os << typ.info;
			for (auto i = std::size_t{}; i < typ.rank; ++i) {
				os << "[]";
			}
			return os;
		}

	}  // namespace sem

}  // namespace minijava
