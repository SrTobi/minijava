/**
 * @file ast_id_checker.hpp
 *
 * @brief
 *     Tools to check that nodes in an AST have proper IDs.
 *
 */

#pragma once

#include <stdexcept>
#include <vector>

#include "parser/for_each_node.hpp"


namespace testaux
{

	/**
	 * @brief
	 *     An AST visitor that collects all IDs form an AST.
	 *
	 * The order in which IDs are collected is unspecified.
	 *
	 */
	class ast_id_collector final : public minijava::for_each_node
	{
	public:

		/**
		 * @brief
		 *     Creates a visitor that will collect the seen IDs into the vector
		 *     `ids`.
		 *
		 * @param ids
		 *     destination for the collected IDs
		 *
		 */
		ast_id_collector(std::vector<std::size_t>& ids);

		/**
		 * @brief
		 *     Collects the ID of a node.
		 *
		 * @param node
		 *     node to have its ID collected
		 *
		 */
		void visit_node(const minijava::ast::node& node) override;

	private:

		/** @brief collected IDs */
		std::vector<std::size_t> * _ids{};

	};  // class ast_id_collector


	/**
	 * @brief
	 *     Checks that the nodes in an AST have IDs of the form 1, 2, &hellip;
	 *     <i>N</i>.
	 *
	 * @param ast
	 *     AST to check
	 *
	 * @throws std::runtime_error
	 *     if the IDs are not of the expected form
	 *
	 */
	void check_ids_strict(const minijava::ast::node& ast);

	/**
	 * @brief
	 *     Checks that the nodes in an AST have non-duplicate non-zero IDs.
	 *
	 * @param ast
	 *     AST to check
	 *
	 * @throws std::runtime_error
	 *     if the IDs are not of the expected form
	 *
	 */
	void check_ids_weak(const minijava::ast::node& ast);

	/**
	 * @brief
	 *     Checks that the all nodes in an AST have ID zero.
	 *
	 * @param ast
	 *     AST to check
	 *
	 * @throws std::runtime_error
	 *     if the IDs are not of the expected form
	 *
	 */
	void check_ids_zero(const minijava::ast::node& ast);

}  // namespace testaux
