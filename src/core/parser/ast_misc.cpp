#include "parser/ast_misc.hpp"

#include <sstream>

#include "parser/pretty_printer.hpp"


namespace minijava
{
	namespace ast
	{

		std::ostream& operator<<(std::ostream& os, const node& ast)
		{
			pretty_printer pp{os};
			ast.accept(pp);
			return os;
		}

		std::string to_string(const node& ast)
		{
			std::ostringstream oss{};
			oss << ast;
			return oss.str();
		}

		bool operator==(const node& lhs, const node& rhs)
		{
			// TODO: Use a more reliable way to compare the ASTs.
			const auto lhs_text = to_string(lhs);
			const auto rhs_text = to_string(rhs);
			return (lhs_text == rhs_text);
		}

		bool operator!=(const node& lhs, const node& rhs)
		{
			return !(lhs == rhs);
		}

	}  // namespace ast

}  // namespace minijava
