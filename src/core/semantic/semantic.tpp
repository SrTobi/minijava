#ifndef MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
#error "Never `#include <semantic/semantic.tpp>` directly; `#include <semantic/semantic.hpp>` instead."
#endif

#include <algorithm>
#include <memory>

#include "exceptions.hpp"
#include "semantic/builtins.hpp"
#include "semantic/constant.hpp"
#include "semantic/dont_use_main_args.hpp"
#include "semantic/name_type_analysis.hpp"
#include "semantic/ref_type_analysis.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/symbol_def.hpp"
#include "semantic/thou_shalt_return.hpp"
#include "semantic/type_info.hpp"
#include "semantic/unique_entry_point.hpp"


namespace minijava
{

	namespace sem
	{

		namespace detail
		{

			template<typename PoolT>
			std::unique_ptr<ast::program> make_builtin_ast(PoolT& pool, ast_factory& factory)
			{
				auto println_arg = factory.make<ast::var_decl>()(
						factory.make<ast::type>()(
								ast::primitive_type::type_int
						),
						pool.normalize(".")
				);
				std::vector<std::unique_ptr<ast::var_decl>> args{};
				args.push_back(std::move(println_arg));
				auto println = factory.make<ast::instance_method>()(
						pool.normalize("println"),
						factory.make<ast::type>()(
								ast::primitive_type::type_void
						),
						std::move(args),
						factory.make<ast::block>()(
								std::vector<std::unique_ptr<ast::block_statement>>{}
						)
				);
				std::vector<std::unique_ptr<ast::instance_method>> methods{};
				methods.push_back(std::move(println));
				auto print_class = factory.make<ast::class_declaration>()(
						pool.normalize("java.io.PrintStream"),
						std::vector<std::unique_ptr<ast::var_decl>>{},
						std::move(methods),
						std::vector<std::unique_ptr<ast::main_method>>{}
				);
				auto out = factory.make<ast::var_decl>()(
						factory.make<ast::type>()(
								pool.normalize("java.io.PrintStream")
						),
						pool.normalize("out")
				);
				std::vector<std::unique_ptr<ast::var_decl>> fields{};
				fields.push_back(std::move(out));
				auto system_class = factory.make<ast::class_declaration>()(
						pool.normalize("java.lang.System"),
						std::move(fields),
						std::vector<std::unique_ptr<ast::instance_method>>{},
						std::vector<std::unique_ptr<ast::main_method>>{}
				);
				auto string_class = factory.make<ast::class_declaration>()(
						pool.normalize("java.lang.String"),
						std::vector<std::unique_ptr<ast::var_decl>>{},
						std::vector<std::unique_ptr<ast::instance_method>>{},
						std::vector<std::unique_ptr<ast::main_method>>{}
				);
				std::vector<std::unique_ptr<ast::class_declaration>> clazzes{};
				clazzes.push_back(std::move(print_class));
				clazzes.push_back(std::move(system_class));
				clazzes.push_back(std::move(string_class));
				return factory.make<ast::program>()(std::move(clazzes));
			}

			template<typename PoolT>
			type_definitions make_non_class_types(PoolT& pool)
			{
				auto result = type_definitions{};
				result.insert(std::make_pair(
						pool.normalize("null"),
						sem::basic_type_info::make_null_type()
				));
				result.insert(std::make_pair(
						pool.normalize("void"),
						sem::basic_type_info::make_void_type()
				));
				result.insert(std::make_pair(
						pool.normalize("int"),
						sem::basic_type_info::make_int_type()
				));
				result.insert(std::make_pair(
						pool.normalize("boolean"),
						sem::basic_type_info::make_boolean_type()
				));
				return result;
			}

			template<typename AllocT>
			globals_map make_globals(symbol_pool<AllocT>& pool)
			{
				sem::globals_map result{};
				result.insert(std::make_pair(
						pool.normalize("System"),
						pool.normalize("java.lang.System")
				));
				return result;
			}

		}  // namespace detail

	}  // namespace sem

	template<typename PoolT>
	semantic_info check_program(const ast::program& ast, PoolT& pool, ast_factory& factory)
	{
		auto builtin_ast = sem::detail::make_builtin_ast(pool, factory);
		auto globals = sem::detail::make_globals(pool);
		auto classes = sem::type_definitions{};
		sem::extract_type_info(*builtin_ast, true, classes);
		sem::extract_type_info(ast, false, classes);
		auto type_annotations = sem::type_attributes{};
		auto locals_annotations = sem::locals_attributes{};
		auto vardecl_annotations = sem::vardecl_attributes{};
		auto method_annotations = sem::method_attributes{};
		sem::perform_shallow_type_analysis(*builtin_ast, classes, type_annotations);
		sem::perform_full_name_type_analysis(
				ast,
				classes,
				globals,
				type_annotations,
				locals_annotations,
				vardecl_annotations,
				method_annotations
		);
		auto const_annotations = sem::extract_constants(ast);
		return semantic_info{
				std::move(classes),
				std::move(type_annotations),
				std::move(locals_annotations),
				std::move(vardecl_annotations),
				std::move(method_annotations),
				std::move(const_annotations),
				std::move(builtin_ast)
		};
	}

	template<typename AllocT>
	void analyze_ast(const ast::program&, symbol_pool<AllocT>&)
	{
		MINIJAVA_THROW_ICE(internal_compiler_error);
	}

}  // namespace minijava
