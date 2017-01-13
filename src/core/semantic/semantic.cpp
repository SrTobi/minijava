#include "semantic/semantic.hpp"

#include <algorithm>
#include <cassert>
#include <functional>


namespace minijava
{

	namespace /* anonymous */
	{

		template<typename T>
		struct mem_comparator
		{
			using comparator = std::less<const T*>;

			bool operator()(const std::unique_ptr<T>& lhs,
			                const std::unique_ptr<T>& rhs) const
			{
				return comparator{}(lhs.get(), rhs.get());
			}

			bool operator()(const T* lhs,
			                const std::unique_ptr<T>& rhs) const
			{
				return comparator{}(lhs, rhs.get());
			}

			bool operator()(const std::unique_ptr<T>& lhs,
			                const T* rhs) const
			{
				return comparator{}(lhs.get(), rhs);
			}
		};

	}  // namespace /* anonymous */


	semantic_info::semantic_info(class_definitions classes,
	                             type_attributes type_annotations,
	                             locals_attributes locals_annotations,
	                             vardecl_attributes vardecl_annotations,
	                             method_attributes method_annotations,
	                             const_attributes const_annotations,
	                             std::unique_ptr<ast::program> builtin_ast,
	                             globals_vector globals)
		: _classes{std::move(classes)}
		, _type_annotations{std::move(type_annotations)}
		, _locals_annotations{std::move(locals_annotations)}
		, _vardecl_annotations{std::move(vardecl_annotations)}
		, _method_annotations{std::move(method_annotations)}
		, _const_annotations{std::move(const_annotations)}
		, _builtin_ast{std::move(builtin_ast)}
		, _globals{std::move(globals)}
	{
		assert(_builtin_ast);
		std::sort(
				std::begin(_globals), std::end(_globals),
				mem_comparator<ast::var_decl>{}
		);
	}

	bool semantic_info::is_global(const ast::var_decl* declaration) const
	{
		assert(declaration);
		return std::binary_search(
				std::begin(_globals), std::end(_globals), declaration,
				mem_comparator<ast::var_decl>{}
		);
	}

}  // namespace minijava
