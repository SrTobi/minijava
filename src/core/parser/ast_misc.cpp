#include "parser/ast_misc.hpp"

#include <algorithm>
#include <ios>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

#include "exceptions.hpp"
#include "parser/pretty_printer.hpp"


namespace minijava
{

	namespace /* anonymous */
	{

		class xml_printer final : public ast::visitor
		{
		public:

			xml_printer(const std::size_t depth = 0) : _depth{depth}
			{
				_stream << std::boolalpha;
			}

			std::string get()
			{
				return _stream.str();
			}

		private:

			std::ostringstream _stream{};

			std::size_t _depth{};

			std::ostream& _indent()
			{
				for (auto i = std::size_t{}; i < _depth; ++i) {
					_stream << '\t';
				}
				return _stream;
			}

			auto _guard(std::string name)
			{
				auto del = [n = std::move(name)](xml_printer* p){
					p->_depth -= 1;
					p->_indent() << "</" << n << ">\n";
				};
				_depth += 1;
				return std::unique_ptr<xml_printer, decltype(del)>{this, std::move(del)};
			}

			template <typename... Ts>
			auto _nest(const std::string& name, Ts&&... args)
			{
				_indent() << "<" << name;
				std::initializer_list<int> dummy = {
					((void)(_stream << " " << args.first << "='" << args.second << "'"), 0)...
				};
				(void) dummy;
				_stream << ">\n";
				return _guard(name);
			}

			template <typename... Ts>
			auto _nest_empty(const std::string& name, Ts&&... args)
			{
				_indent() << "<" << name;
				std::initializer_list<int> dummy = {
					((void)(_stream << " " << args.first << "='" << args.second << "'"), 0)...
				};
				(void) dummy;
				_stream << " />\n";
			}

			void visit_node(const ast::node& /* node */) override
			{
				MINIJAVA_NOT_REACHED();
			}

			void visit(const ast::type& node) override
			{
				const auto name = node.name();
				auto fancy = std::string{};
				auto primitive = true;
				if (const auto p = boost::get<symbol>(&name)) {
					fancy = p->c_str();
					primitive = false;
				} else {
					switch (boost::get<ast::primitive_type>(name)) {
					case ast::primitive_type::type_int:
						fancy = "int";
						break;
					case ast::primitive_type::type_boolean:
						fancy = "boolean";
						break;
					case ast::primitive_type::type_void:
						fancy = "void";
						break;
					}
				}
				assert(!fancy.empty());
				_nest_empty(
					"type",
					std::make_pair("name", fancy),
					std::make_pair("primitive", primitive),
					std::make_pair("rank", node.rank())
				);
			}

			void visit(const ast::var_decl& node) override
			{
				const auto g = _nest("var-decl", std::make_pair("name", node.name()));
				node.var_type().accept(*this);
			}

			void visit(const ast::binary_expression& node) override
			{
				static const auto ops = std::map<ast::binary_operation_type, std::string>{
					{ast::binary_operation_type::assign,        "ASSIGN"},
					{ast::binary_operation_type::logical_or,    "LOGICAL_OR"},
					{ast::binary_operation_type::logical_and,   "LOGICAL_AND"},
					{ast::binary_operation_type::equal,         "EQUAL"},
					{ast::binary_operation_type::not_equal,     "NOT_EQUAL"},
					{ast::binary_operation_type::less_than,     "LESS_THAN"},
					{ast::binary_operation_type::less_equal,    "LESS_EQUAL"},
					{ast::binary_operation_type::greater_than,  "GREATER_THAN"},
					{ast::binary_operation_type::greater_equal, "GREATER_EQUAL"},
					{ast::binary_operation_type::plus,          "PLUS"},
					{ast::binary_operation_type::minus,         "MINUS"},
					{ast::binary_operation_type::multiply,      "MULTIPLY"},
					{ast::binary_operation_type::divide,        "DIVIDE"},
					{ast::binary_operation_type::modulo,        "MODULO"},
				};
				const auto g1 = _nest(
					"binary-expression",
					std::make_pair("operation", ops.at(node.type()))
				);
				{
					const auto g2 = _nest("lhs");
					node.lhs().accept(*this);
				}
				{
					const auto g2 = _nest("rhs");
					node.rhs().accept(*this);
				}
			}

			void visit(const ast::unary_expression& node) override
			{
				static const auto ops = std::map<ast::unary_operation_type, std::string>{
					{ast::unary_operation_type::logical_not, "LOGICAL_NOT"},
					{ast::unary_operation_type::minus,       "MINUS"},
				};
				const auto g1 = _nest(
					"unary-expression",
					std::make_pair("operation", ops.at(node.type()))
				);
				{
					const auto g2 = _nest("target");
					node.target().accept(*this);
				}
			}

			void visit(const ast::object_instantiation& node) override
			{
				_nest_empty(
					"object-instantiation",
					std::make_pair("class", node.class_name())
				);
			}

			void visit(const ast::array_instantiation& node) override
			{
				const auto g1 = _nest("array-instantiation");
				{
					const auto g2 = _nest("type");
					node.array_type().accept(*this);
				}
				{
					const auto g2 = _nest("extent");
					node.extent().accept(*this);
				}
			}

			void visit(const ast::array_access& node) override
			{
				const auto g1 = _nest("array-access");
				{
					const auto g2 = _nest("target");
					node.target().accept(*this);
				}
				{
					const auto g2 = _nest("index");
					node.index().accept(*this);
				}
			}

			void visit(const ast::variable_access& node) override
			{
				const auto g1 = _nest(
					"variable-access",
					std::make_pair("name", node.name())
				);
				if (const auto tgtptr = node.target()) {
					const auto g2 = _nest("target");
					tgtptr->accept(*this);
				} else {
					_nest_empty("target");
				}
			}

			void visit(const ast::method_invocation& node) override
			{
				const auto g1 = _nest(
					"method-invocation",
					std::make_pair("name", node.name())
				);
				if (const auto p = node.target()) {
					const auto g2 = _nest("target");
					p->accept(*this);
				} else {
					_nest_empty("target");
				}
				{
					const auto g2 = _nest("arguments");
					for (auto&& child : node.arguments()) {
						child->accept(*this);
					}
				}
			}

			void visit(const ast::this_ref& /* node */) override
			{
				_nest_empty("this-ref");
			}

			void visit(const ast::boolean_constant& node) override
			{
				_nest_empty(
					"boolean-constant",
					std::make_pair("value", node.value())
				);
			}

			void visit(const ast::integer_constant& node) override
			{
				_nest_empty(
					"integer-constant",
					std::make_pair("literal", node.literal()),
					std::make_pair("negative", node.negative())
				);
			}

			void visit(const ast::null_constant& /* node */) override
			{
				_nest_empty("null-constant");
			}

			void visit(const ast::local_variable_statement& node) override
			{
				const auto g1 = _nest("local-variable-statement");
				{
					const auto g2 = _nest("declaration");
					node.declaration().accept(*this);
				}
				if (const auto p = node.initial_value()) {
					const auto g2 = _nest("initial-value");
					p->accept(*this);
				} else {
					_nest_empty("initial-value");
				}
			}

			void visit(const ast::expression_statement& node) override
			{
				const auto g1 = _nest("expression-statement");
				const auto g2 = _nest("inner-expression");
				node.inner_expression().accept(*this);
			}

			void visit(const ast::block& node) override
			{
				const auto g1 = _nest("block");
				const auto g2 = _nest("body");
				for (auto&& child : node.body()) {
					child->accept(*this);
				}
			}

			void visit(const ast::if_statement& node) override
			{
				const auto g1 = _nest("if-statement");
				{
					const auto g2 = _nest("condition");
					node.condition().accept(*this);
				}
				{
					const auto g2 = _nest("then");
					node.then_statement().accept(*this);
				}
				if (const auto p = node.else_statement()) {
					const auto g2 = _nest("else");
					p->accept(*this);
				} else {
					_nest_empty("else");
				}
			}

			void visit(const ast::while_statement& node) override
			{
				const auto g1 = _nest("while-statement");
				{
					const auto g2 = _nest("condition");
					node.condition().accept(*this);
				}
				{
					const auto g2 = _nest("body");
					node.body().accept(*this);
				}
			}

			void visit(const ast::return_statement& node) override
			{
				const auto g1 = _nest("return-statement");
				if (const auto p = node.value()) {
					const auto g2 = _nest("value");
					p->accept(*this);
				} else {
					_nest_empty("value");
				}
			}

			void visit(const ast::empty_statement& /* node */) override
			{
				_nest_empty("empty-statement");
			}

			void visit(const ast::main_method& node) override
			{
				const auto g1 = _nest(
					"main-method",
					std::make_pair("name", node.name()),
					std::make_pair("argname", node.argname())
				);
				const auto g2 = _nest("body");
				node.body().accept(*this);
			}

			void visit(const ast::instance_method& node) override
			{
				const auto g1 = _nest(
					"instance-method",
					std::make_pair("name", node.name())
				);
				{
					const auto g1 = _nest("return-type");
					node.return_type().accept(*this);
				}
				{
					const auto g1 = _nest("parameters");
					for (auto&& child : node.parameters()) {
						child->accept(*this);
					}
				}
				{
					const auto g1 = _nest("body");
					node.body().accept(*this);
				}
			}

			void visit(const ast::class_declaration& node) override
			{
				const auto g1 = _nest(
					"class-declaration",
					std::make_pair("name", node.name())
				);
				{
					const auto g2 = _nest("fields");
					_serialize_in_deterministic_order(node.fields());
				}
				{
					const auto g2 = _nest("instance-methods");
					_serialize_in_deterministic_order(node.instance_methods());
				}
				{
					const auto g2 = _nest("main-methods");
					_serialize_in_deterministic_order(node.main_methods());
				}
			}

			void visit(const ast::program& node) override
			{
				const auto g1 = _nest("program");
				const auto g2 = _nest("classes");
				_serialize_in_deterministic_order(node.classes());
			}

			template <typename ContainerT>
			void _serialize_in_deterministic_order(const ContainerT& children)
			{
				auto texts = std::vector<std::string>{};
				texts.reserve(children.size());
				std::transform(
					std::begin(children), std::end(children),
					std::back_inserter(texts),
					[depth = this->_depth](auto&& n){
						xml_printer v{depth};
						n->accept(v);
						return v.get();
					}
				);
				std::sort(std::begin(texts), std::end(texts));
				std::copy(
					std::begin(texts), std::end(texts),
					std::ostream_iterator<std::string>(_stream)
				);
			}

		};  // class xml_printer

	}  // namespace /* anonymous */

	namespace ast
	{

		std::ostream& operator<<(std::ostream& os, const node& ast)
		{
			pretty_printer pp{os};
			ast.accept(pp);
			return os;
		}

		std::string to_text(const node& ast)
		{
			std::ostringstream oss{};
			oss << ast;
			return oss.str();
		}

		std::string to_xml(const node& ast)
		{
			xml_printer jp{};
			ast.accept(jp);
			return jp.get();
		}

		bool operator==(const node& lhs, const node& rhs)
		{
			return (to_xml(lhs) == to_xml(rhs));
		}

		bool operator!=(const node& lhs, const node& rhs)
		{
			return !(lhs == rhs);
		}

	}  // namespace ast

}  // namespace minijava
