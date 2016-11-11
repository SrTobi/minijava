#pragma once

#include <ostream>

#include "parser/ast.hpp"

namespace minijava
{
	namespace ast
	{
		/**
		 * AST pretty printer
		 */
		class pretty_printer : public visitor
		{
		public:

			/**
			 * @brief
			 *     Constructs a new pretty printer which writes to the given
			 *     output stream.
			 *
			 * @param output
			 *     output stream
			 *
			 */
			pretty_printer(std::ostream& output)
					: _output{output} {}

		private:

			/** @brief output stream */
			std::ostream& _output;

			//std::size_t indentation_level;
			// ...
		};
	}
}