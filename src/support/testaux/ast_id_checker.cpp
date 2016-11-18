#include "testaux/ast_id_checker.hpp"

#include <stdexcept>


namespace testaux
{

	ast_id_collector::ast_id_collector(std::vector<std::size_t>& ids) : _ids{&ids}
	{
	}

	void ast_id_collector::visit_node(const minijava::ast::node& node)
	{
		_ids->push_back(node.id());
	}

	void check_ids_strict(const minijava::ast::node& ast)
	{
		auto ids = std::vector<std::size_t>{};
		auto vst = ast_id_collector{ids};
		ast.accept(vst);
		const auto first = std::begin(ids);
		const auto last = std::end(ids);
		std::sort(first, last);
		for (auto i = std::size_t{}; i < ids.size(); ++i) {
			if (ids[i] != i + 1) {
				throw std::runtime_error{"IDs not of form 1, 2, ..., N"};
			}
		}
	}

	void check_ids_weak(const minijava::ast::node& ast)
	{
		auto ids = std::vector<std::size_t>{};
		auto vst = ast_id_collector{ids};
		ast.accept(vst);
		const auto first = std::begin(ids);
		const auto last = std::end(ids);
		std::sort(first, last);
		if (std::find(first, last, 0) != last) {
			throw std::runtime_error{"AST contains node with ID 0"};
		}
		if (std::adjacent_find(first, last) != last) {
			throw std::runtime_error{"AST contains duplicate IDs"};
		}
	}

	void check_ids_zero(const minijava::ast::node& ast)
	{
		auto ids = std::vector<std::size_t>{};
		auto vst = ast_id_collector{ids};
		ast.accept(vst);
		const auto first = std::begin(ids);
		const auto last = std::end(ids);
		if (!std::all_of(first, last, [](auto i){ return i == 0; })) {
			throw std::runtime_error{"AST contains nodes with non-zero IDs"};
		}
	}

}  // namespace testaux
