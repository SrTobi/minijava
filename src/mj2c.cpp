#include "cli.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>

#include "exceptions.hpp"
#include "io/file_data.hpp"
#include "io/file_output.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token_iterator.hpp"
#include "parser/parser.hpp"
#include "runtime/runtime.hpp"
#include "semantic/semantic.hpp"
#include "symbol/symbol_pool.hpp"


using namespace minijava;

namespace /* anonymous */
{

	std::string replace_dots(const std::string& name)
	{
		auto better = std::string{};
		std::transform(
			std::begin(name), std::end(name),
			std::back_inserter(better),
			[](const auto c){ return (c == '.') ? '_' : c; }
		);
		return better;
	}

	std::string mangle_class(const std::string& name)
	{
		return "Mj" + std::to_string(name.length()) + "_" + replace_dots(name);
	}

	std::string mangle_class(const symbol name)
	{
		return mangle_class(std::string{name.c_str(), name.length()});
	}

	std::string mangle_method(const std::string& class_name, const std::string& method_name)
	{
		return mangle_class(class_name)
			+ "_" + replace_dots(method_name)
			+ "_" + std::to_string(method_name.length());
	}

	std::string mangle_method(const symbol class_name, const symbol method_name)
	{
		return mangle_method(
			std::string{class_name.c_str(), class_name.length()},
			std::string{method_name.c_str(), method_name.length()}
		);
	}

	std::string mangle_variable(const std::string& name)
	{
		using namespace std::string_literals;
		return "mj" + std::to_string(name.length()) + "_" + replace_dots(name);
	}

	std::string mangle_variable(const symbol name)
	{
		return mangle_variable(std::string{name.c_str(), name.length()});
	}

	std::string get_formatted_type_name(const ast::type& type)
	{
		auto type_name = std::string{};
		if (const auto p = boost::get<ast::primitive_type>(&type.name())) {
			switch (*p) {
			case ast::primitive_type::type_int:
				type_name = "int32_t";
				break;
			case ast::primitive_type::type_boolean:
				type_name = "bool";
				break;
			case ast::primitive_type::type_void:
				type_name = "void";
				break;
			}
		} else {
			type_name = "struct " + mangle_class(boost::get<symbol>(type.name()));
			type_name.push_back('*');
		}
		for (auto i = std::size_t{}; i < type.rank(); ++i) {
			type_name.push_back('*');
		}
		return type_name;
	}

	void print_method_signature(const symbol classname,
								const ast::instance_method& method,
								file_output& out)
	{
		const auto name = mangle_method(classname, method.name());
		const auto type = get_formatted_type_name(method.return_type());
		out.print(
			"static %s %s(struct %s* THIS",
			type.c_str(), name.c_str(), mangle_class(classname).c_str()
		);
		for (auto&& param : method.parameters()) {
			out.print(
				", %s %s",
				get_formatted_type_name(param->var_type()).c_str(),
				mangle_variable(param->name()).c_str()
			);
		}
		out.write(")");
	}


	class trans_c_visitor final : public ast::visitor
	{
	public:

		trans_c_visitor(const semantic_info& seminfo, file_output& out)
			: _seminfo{seminfo}, _out{out}
		{
		}

	private:

		using ast::visitor::visit;

		const semantic_info& _seminfo;
		file_output& _out;
		const ast::class_declaration* _current_class{};
		const ast::instance_method* _current_method{};
		std::string _indent{};
		bool _parenthesize_expressions{};

		void _open_brace()
		{
			_out.print("%s{\n", _indent.c_str());
			_indent.push_back('\t');
		}

		void _close_brace()
		{
			assert(!_indent.empty());
			_indent.pop_back();
			_out.print("%s}\n", _indent.c_str());
		}

		auto _nest_braces()
		{
			const auto del = [](trans_c_visitor* tcv){ tcv->_close_brace(); };
			_open_brace();
			return std::unique_ptr<trans_c_visitor, decltype(del)>{this, del};
		}

		auto _descend_into_expression()
		{
			const auto del = [old = this->_parenthesize_expressions](trans_c_visitor* tcv){
				tcv->_parenthesize_expressions = old;
				if (old) { tcv->_out.write(")"); }
			};
			if (_parenthesize_expressions) {
				_out.write("(");
			}
			_parenthesize_expressions = true;
			return std::unique_ptr<trans_c_visitor, decltype(del)>{this, del};
		}

		auto _require_parens()
		{
			const auto del = [old = this->_parenthesize_expressions](trans_c_visitor* tcv){
				tcv->_parenthesize_expressions = old;
			};
			_parenthesize_expressions = true;
			return std::unique_ptr<trans_c_visitor, decltype(del)>{this, del};
		}

		void visit_node(const ast::node& /* node */) override
		{
			MINIJAVA_NOT_REACHED();
		}

		void visit_expression(const ast::expression& node) override
		{
			const auto it = _seminfo.const_annotations().find(&node);
			if (it != _seminfo.const_annotations().end()) {
				const long value = it->second;
				_out.print("%ld", value);
			} else {
				node.accept(*this);
			}
		}

		void visit(const ast::type& node) override
		{
			_out.write(get_formatted_type_name(node));
		}

		void visit(const ast::var_decl& node) override
		{
			node.var_type().accept(*this);
			_out.write(" ");
			_out.write(mangle_variable(node.name()));
		}

		void visit(const ast::binary_expression& node) override
		{
			static const auto ops = std::map<ast::binary_operation_type, std::string>{
				{ast::binary_operation_type::assign,        "=" },
				{ast::binary_operation_type::logical_or,    "||"},
				{ast::binary_operation_type::logical_and,   "&&"},
				{ast::binary_operation_type::equal,         "=="},
				{ast::binary_operation_type::not_equal,     "!="},
				{ast::binary_operation_type::less_than,     "<" },
				{ast::binary_operation_type::less_equal,    "<="},
				{ast::binary_operation_type::greater_than,  ">" },
				{ast::binary_operation_type::greater_equal, ">="},
				{ast::binary_operation_type::plus,          "+" },
				{ast::binary_operation_type::minus,         "-" },
				{ast::binary_operation_type::multiply,      "*" },
				{ast::binary_operation_type::divide,        "/" },
				{ast::binary_operation_type::modulo,        "%" },
			};
			const auto g = _descend_into_expression();
			visit_expression(node.lhs());
			_out.print(" %s ", ops.at(node.type()).c_str());
			visit_expression(node.rhs());
		}

		void visit(const ast::unary_expression& node) override
		{
			static const auto ops = std::map<ast::unary_operation_type, std::string>{
				{ast::unary_operation_type::logical_not, "!"},
				{ast::unary_operation_type::minus,       "-"},
			};
			const auto g = _descend_into_expression();
			_out.write(ops.at(node.type()));
			visit_expression(node.target());
		}

		void visit(const ast::object_instantiation& node) override
		{
			_out.print(
				"mj_runtime_allocate(1, (int32_t) sizeof(struct %s))",
				mangle_class(node.class_name()).c_str()
			);
		}

		void visit(const ast::array_instantiation& node) override
		{
			auto member_type = get_formatted_type_name(node.array_type());
			assert(member_type.back() == '*');
			member_type.pop_back();
			_out.write("mj_runtime_allocate(");
			visit_expression(node.extent());
			_out.write(", ");
			_out.print("(int32_t) sizeof(%s)", member_type.c_str());
			_out.write(")");
		}

		void visit(const ast::array_access& node) override
		{
			{
				const auto g = _require_parens();
				visit_expression(node.target());
			}
			_out.write("[");
			visit_expression(node.index());
			_out.write("]");
		}

		void visit(const ast::variable_access& node) override
		{
			const auto decl = _seminfo.vardecl_annotations().at(node);
			if (const auto p = node.target()) {
				visit_expression(*p);
				_out.write("->");
			} else if ((_current_method != nullptr) && !_seminfo.is_global(decl)) {
				const auto it = [decl](const auto& locals) {
					return std::find(std::begin(locals), std::end(locals), decl);
				}(_seminfo.locals_annotations().at(*_current_method));
				if (it == decltype(it){}) {
					_out.write("THIS->");
				}
			}
			_out.write(mangle_variable(node.name()));
		}

		void visit(const ast::method_invocation& node) override
		{
			if (const auto p = node.target()) {
				const auto target_type = _seminfo.type_annotations().at(*p);
				assert(target_type.rank == 0);
				const auto mangled = mangle_method(
					target_type.info.declaration()->name(),
					node.name()
				);
				_out.write(mangled);
				_out.write("(");
				visit_expression(*p);
			} else {
				assert(_current_class != nullptr);
				const auto mangled = mangle_method(
					_current_class->name(),
					node.name()
				);
				_out.print("%s(THIS", mangled.c_str());
			}
			for (auto&& child : node.arguments()) {
				_out.write(", ");
				visit_expression(*child);
			}
			_out.write(")");
		}

		void visit(const ast::this_ref& /* node */) override
		{
			_out.write("THIS");
		}

		void visit(const ast::null_constant& /* node */) override
		{
			// We could use `NULL` but it would be kind of silly to
			// `#include <stdlib.h>` only for that.
			_out.write("0");
		}

		void visit(const ast::local_variable_statement& node) override
		{
			_out.write(_indent);
			node.declaration().accept(*this);
			if (const auto p = node.initial_value()) {
				_out.write(" = ");
				visit_expression(*p);
			}
			_out.write(";\n");
		}

		void visit(const ast::expression_statement& node) override
		{
			_out.write(_indent);
			visit_expression(node.inner_expression());
			_out.write(";\n");
		}

		void visit(const ast::block& node) override
		{
			const auto g = _nest_braces();
			for (auto&& child : node.body()) {
				child->accept(*this);
			}
		}

		void visit(const ast::if_statement& node) override
		{
			_out.print("%sif (", _indent.c_str());
			visit_expression(node.condition());
			_out.write(")\n");
			_visit_clause(node.then_statement());
			if (const auto p = node.else_statement()) {
				_out.print("%selse\n", _indent.c_str());
				_visit_clause(*p);
			}
		}

		void visit(const ast::while_statement& node) override
		{
			_out.print("%swhile (", _indent.c_str());
			visit_expression(node.condition());
			_out.write(")\n");
			_visit_clause(node.body());
		}

		void visit(const ast::return_statement& node) override
		{
			_out.print("%sreturn", _indent.c_str());
			if (const auto p = node.value()) {
				_out.write(" ");
				visit_expression(*p);
			}
			_out.write(";\n");
		}

		void visit(const ast::empty_statement& /* node */) override
		{
			_out.print("%s;\n", _indent.c_str());
		}

		void visit(const ast::main_method& node) override
		{
			const auto system_class_name = mangle_class("java.lang.System");
			const auto system_name = mangle_variable("System");
			const auto printstream_class_name = mangle_class("java.io.PrintStream");
			const auto out_name = mangle_variable("out");
			_out.write("void minijava_main(void)\n");
			const auto g = _nest_braces();
			_out.print(
				"%s%s = mj_runtime_allocate(1, (int32_t) sizeof(struct %s));\n",
				_indent.c_str(), system_name.c_str(), system_class_name.c_str()
			);
			_out.print(
				"%s%s->%s = mj_runtime_allocate(1, (int32_t) sizeof(struct %s));\n",
				_indent.c_str(), system_name.c_str(), out_name.c_str(),
				printstream_class_name.c_str()
			);
			node.body().accept(*this);
		}

		void visit(const ast::instance_method& node) override
		{
			const auto del = [](trans_c_visitor* tcv){ tcv->_current_method = nullptr; };
			const auto guard = std::unique_ptr<trans_c_visitor, decltype(del)>{this, del};
			_current_method = &node;
			print_method_signature(_current_class->name(), node, _out);
			_out.write("\n");
			node.body().accept(*this);
			_out.write("\n");
		}

		void visit(const ast::class_declaration& node) override
		{
			const auto del = [](trans_c_visitor* tcv){ tcv->_current_class = nullptr; };
			const auto guard = std::unique_ptr<trans_c_visitor, decltype(del)>{this, del};
			_current_class = &node;
			for (auto&& child : node.instance_methods()) {
				child->accept(*this);
			}
			for (auto&& child : node.main_methods()) {
				child->accept(*this);
			}
		}

		void visit(const ast::program& node) override
		{
			for (auto&& child : node.classes()) {
				child->accept(*this);
			}
		}

		void _visit_clause(const ast::statement& body)
		{
			const auto g = _nest_braces();
			if (const auto p = dynamic_cast<const ast::block*>(&body)) {
				for (auto&& child : p->body()) {
					child->accept(*this);
				}
			} else {
				body.accept(*this);
			}
		}

	};  // class trans_c_visitor


	void to_c(const ast::program& ast, const semantic_info& seminfo, file_output& out)
	{
		out.write(runtime_source());
		out.write("\n");
		out.write("#include <stdbool.h>\n");
		out.write("#include <stddef.h>\n");
		out.write("#include <stdint.h>\n");
		out.write("\n");
		for (auto&& clazz : seminfo.classes()) {
			out.print("struct %s;\n", mangle_class(clazz.first).c_str());
		}
		out.write("\n");
		for (auto&& clazz : seminfo.classes()) {
			const auto cls_mangled = mangle_class(clazz.first);
			out.print("struct %s\n", cls_mangled.c_str());
			out.write("{\n");
			for (auto&& field : clazz.second.declaration()->fields()) {
				const auto name = mangle_variable(field->name());
				const auto type = get_formatted_type_name(field->var_type());
				out.print("\t%s %s;\n", type.c_str(), name.c_str());
			}
			out.write("};\n\n");
			for (auto&& method : clazz.second.declaration()->instance_methods()) {
				print_method_signature(clazz.first, *method, out);
				out.write(";\n\n");
			}
		}
		const auto system_class_name = mangle_class("java.lang.System");
		const auto system_name = mangle_variable("System");
		const auto printstream_class_name = mangle_class("java.io.PrintStream");
		const auto out_name = mangle_variable("out");
		const auto println_name = mangle_method("java.io.PrintStream", "println");
		const auto param_name = mangle_variable(".");
		out.print(
			"static struct %s* %s;\n\n",
			system_class_name.c_str(), system_name.c_str()
		);
		out.print(
			"static void %s(struct %s* THIS, int32_t %s)\n",
			println_name.c_str(), printstream_class_name.c_str(),
			param_name.c_str()
		);
		out.write("{\n");
		out.print("\tmj_runtime_println(%s);\n", param_name.c_str());
		out.write("}\n\n");
		trans_c_visitor tcv{seminfo, out};
		ast.accept(tcv);
	}

	void translate(file_data& in, file_output& out)
	{
		auto pool = symbol_pool<>{};
		auto lex = make_lexer(std::begin(in), std::end(in), pool, pool);
		const auto tokfirst = token_begin(lex);
		const auto toklast = token_end(lex);
		auto factory = ast_factory{};
		const auto ast = parse_program(tokfirst, toklast, factory);
		in.dispose();
		const auto sem_info = check_program(*ast, pool, factory);
		to_c(*ast, sem_info, out);
		out.flush();
	}

}  // namespace /* anonymous */


int main(int argc, char**)
{
	try {
		if (argc > 1) {
			throw std::invalid_argument{"Too many arguments"};
		}
		auto in = file_data{stdin};
		auto out = file_output{stdout};
		translate(in, out);
	} catch (const std::exception& e) {
		std::fprintf(stderr, "mj2c: error: %s\n", e.what());
		return EXIT_FAILURE;
	}
}
