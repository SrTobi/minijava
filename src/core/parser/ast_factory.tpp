#ifndef MINIJAVA_INCLUDED_FROM_PARSER_AST_FACTORY_HPP
#error "Never `#include <parser/ast_factory.tpp>` directly; `#include <parser/ast_factory.hpp>` instead."
#endif

#include <cassert>
#include "position.hpp"

namespace minijava
{

	template <typename NodeT>
	ast_builder<NodeT>::ast_builder(const std::size_t id) noexcept
	{
		_mutator.id = id;
	}

	template <typename NodeT>
	ast_builder<NodeT>& ast_builder<NodeT>::at_line(const std::size_t line)
	{
		assert((line != 0) && (_mutator.line == 0));
		_mutator.line = line;
		return *this;
	}

	template <typename NodeT>
	ast_builder<NodeT>& ast_builder<NodeT>::at(const minijava::position position)
	{
		assert((position.line() != 0) && (_mutator.line == 0));
		assert((position.column() != 0) && (_mutator.column == 0));
		_mutator.line = position.line();
		_mutator.column = position.column();
		return *this;
	}

	template <typename NodeT>
	ast_builder<NodeT>& ast_builder<NodeT>::at_column(const std::size_t column)
	{
		assert((column != 0) && (_mutator.column == 0));
		_mutator.column = column;
		return *this;
	}

	template <typename NodeT>
	template <typename... ArgTs>
	std::enable_if_t
	<
		std::is_constructible<NodeT, ArgTs...>{},
		std::unique_ptr<NodeT>
	>
	ast_builder<NodeT>::operator()(ArgTs&&... args) const
	{
		auto np = std::make_unique<NodeT>(std::forward<ArgTs>(args)...);
		_mutator(*np);
		return np;
	}

	template <typename NodeT>
	std::enable_if_t<std::is_base_of<ast::node, NodeT>{}, ast_builder<NodeT>>
	ast_factory::make()
	{
		return ast_builder<NodeT>{++_id};
	}

}  // namespace minijava
