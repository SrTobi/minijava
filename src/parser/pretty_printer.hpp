#pragma once

#include <ostream>

#include "parser/ast.hpp"

namespace minijava
{
	namespace ast
	{
		class pretty_printer : public visitor
		{
		public:

			pretty_printer(std::ostream& output)
					: _output{output} {}

		private:

			std::ostream& _output;

			//std::size_t indentation_level;
			// ...
		};
	}
}