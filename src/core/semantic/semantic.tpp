#ifndef MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
#error "Never `#include <semantic/analyze.tpp>` directly; `#include <semantic/analyze.hpp>` instead."
#endif

#include <memory>

#include "semantic/builtins.hpp"
#include "semantic/constant.hpp"
#include "semantic/ref_type_analysis.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/symbol_def.hpp"
#include "semantic/thou_shalt_return.hpp"
#include "semantic/unique_entry_point.hpp"
#include "semantic/dont_use_main_args.hpp"

namespace minijava
{
	namespace sem
	{
		namespace detail
		{
			namespace ast = minijava::ast;

			template<typename AllocT>
			std::unique_ptr<ast::program> make_builtin_ast(
					symbol_pool<AllocT>& pool) {
				auto println_arg = std::make_unique<ast::var_decl>(
						std::make_unique<ast::type>(
								ast::primitive_type::type_int
						),
						pool.normalize(".")
				);
				std::vector<std::unique_ptr<ast::var_decl>> args{};
				args.push_back(std::move(println_arg));
				auto println = std::make_unique<ast::instance_method>(
						pool.normalize("println"),
						std::make_unique<ast::type>(
								ast::primitive_type::type_void
						),
						std::move(args),
						std::make_unique<ast::block>(
								std::vector<ast::block_statement>{}
						)
				);
				std::vector<std::unique_ptr<ast::instance_method>> methods{};
				methods.push_back(std::move(println));
				auto print_class = std::make_unique<ast::class_declaration>(
						pool.normalize("java.io.PrintStream"),
						std::vector<std::unique_ptr<ast::var_decl>>{},
						std::move(methods),
						std::vector<std::unique_ptr<ast::main_method>>{}
				);
				auto out = std::make_unique<ast::var_decl>(
						std::make_unique<ast::type>(
								pool.normalize("java.io.PrintStream")
						),
						pool.normalize("out")
				);
				std::vector<std::unique_ptr<ast::instance_method>> fields{};
				fields.push_back(std::move(out));
				auto system_class = std::make_unique<ast::class_declaration>(
						pool.normalize("java.lang.System"),
						std::move(fields),
						std::vector<std::unique_ptr<ast::instance_method>>{},
						std::vector<std::unique_ptr<ast::main_method>>{}
				);
				auto string_class = std::make_unique<ast::class_declaration>(
						pool.normalize("java.lang.String"),
						std::vector<std::unique_ptr<ast::var_decl>>{},
						std::vector<std::unique_ptr<ast::instance_method>>{},
						std::vector<std::unique_ptr<ast::main_method>>{}
				);
				std::vector<std::unique_ptr<ast::class_declaration>> clazzes{};
				clazzes.push_back(std::move(print_class));
				clazzes.push_back(std::move(system_class));
				clazzes.push_back(std::move(string_class));
				return std::make_unique<ast::program>(std::move(clazzes));
			}
		}
	}

	template<typename AllocT>
	semantic_info check_program(const ast::program& ast,
								symbol_pool<AllocT>& pool)
	{
		auto builtin_ast = sem::detail::make_builtin_ast(pool);
		// FIXME
		(void) builtin_ast;
		throw nullptr;
	}

	template<typename AllocT>
	void analyze_ast(const ast::program& ast, symbol_pool<AllocT>& pool)
	{
		auto defa = semantic::def_annotations{};
		auto typs = semantic::extract_typesystem(ast, defa, pool);
		auto syst = semantic::builtins::register_system(typs, pool);
		auto annotations = semantic::analyse_program(ast, {{pool.normalize("System"), syst}}, typs, defa);
		const auto handler = [](const ast::node& /* n */){
			// FIXME: add source code location to exception object
			// FIXME: actually throw exception (binary_arithmetic.mj is supposed to be valid ATM)
			// throw semantic_error{"Result of expression is undefined"};
		};
		auto constants = extract_constants(ast, handler);
		check_return_paths(ast);
		check_unique_entry_point(ast);
		check_args_usage(ast, defa, annotations.second);
		// FIXME: return everything we want to keep
		(void) annotations;
		(void) constants;
	}
}
