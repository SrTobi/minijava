/**
 * @file ast_misc.hpp
 *
 * @brief
 *     More or less useful operations on ASTs.
 *
 * The functions provided by this file are pretty expensive and should only be
 * used for debugging or when absolutely required.
 *
 */

#pragma once

#include <iosfwd>
#include <string>

#include "parser/ast.hpp"


namespace minijava
{

	namespace ast
	{

		/**
		 * @brief
		 *     &ldquo;Pretty prints&rdquo; an AST to a stream.
		 *
		 * The printed text is as close as possible to the
		 * &ldquo;official&rdquo; format.  This means that serialization is no
		 * bijection.  Two ASTs migt serialize to the same text despite having
		 * different nodes.
		 *
		 * @param os
		 *     stream to write to
		 *
		 * @param ast
		 *     AST to serialize
		 *
		 * @returns
		 *     reference to `os`
		 *
		 */
		std::ostream& operator<<(std::ostream& os, const node& ast);

		/**
		 * @brief
		 *     Serializes an AST in &ldquo;pretty printed&rdquo; format.
		 *
		 * This function has the same effect as the following code, although it
		 * might be implemented differently.
		 *
		 *     std::string to_string(const node& ast)
		 *     {
		 *         std::ostringstream oss{};
		 *         oss << ast;
		 *         return oss.str();
		 *     }
		 *
		 * @param ast
		 *     AST to serialize
		 *
		 * @returns
		 *     serialized AST
		 *
		 */
		std::string to_string(const node& ast);

		/**
		 * @brief
		 *     Compares two ASTs for equality.
		 *
		 * Two ASTs are considered equal if and only if they have exactly the
		 * same nodes and the only non-trivial isomorphism is the ordering of
		 * `instance_method` and `main_method` nodes in `class_declaration`
		 * nodes and the ordering of `class_declaration` nodes in `program`
		 * nodes.  Node IDs and source code location information is ignored
		 * during the comparison.
		 *
		 * This function is well-defined even if both ASTs were constructed
		 * using different symbol pools.
		 *
		 * Be aware that the comparison is an expensive operation and
		 * internally allocates memory.
		 *
		 * @param lhs
		 *     first AST to compare
		 *
		 * @param rhs
		 *     second AST to compare
		 *
		 * @returns
		 *     whether `lhs` and `rhs` are equal
		 *
		 */
		bool operator==(const node& lhs, const node& rhs);

		/**
		 * @brief
		 *     Compares two ASTs for inequality.
		 *
		 * @param lhs
		 *     first AST to compare
		 *
		 * @param rhs
		 *     second AST to compare
		 *
		 * @returns
		 *     `! (lhs == rhs)`
		 *
		 */
		bool operator!=(const node& lhs, const node& rhs);

	}  // namespace ast

}  // namespace minijava
