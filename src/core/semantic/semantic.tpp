#ifndef MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
#error "Never `#include <semantic/semantic.tpp>` directly; `#include <semantic/semantic.hpp>` instead."
#endif

#include <cstdio>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

#include "exceptions.hpp"
#include "semantic/constant.hpp"
#include "semantic/name_type_analysis.hpp"
#include "semantic/semantic_error.hpp"
#include "semantic/thou_shalt_return.hpp"
#include "semantic/type_info.hpp"


namespace minijava
{

	namespace sem
	{

		namespace detail
		{

			template<typename PoolT>
			std::unique_ptr<ast::instance_method>
			make_builtin_method(const char *const name,
			                    const ast::primitive_type rettype,
			                    const std::initializer_list<ast::primitive_type> paramtypes,
			                    PoolT& pool, ast_factory& factory)
			{
				auto args = std::vector<std::unique_ptr<ast::var_decl>>{};
				auto i = 0;
				for (const auto typ : paramtypes) {
					char param[32];
					std::snprintf(param, sizeof(param), "_%u", ++i);
					args.push_back(
						factory.make<ast::var_decl>()(
							factory.make<ast::type>()(typ),
							pool.normalize(param)
						)
					);
				}
				return factory.make<ast::instance_method>()(
					pool.normalize(name),
					factory.make<ast::type>()(rettype),
					std::move(args),
					factory.make<ast::block>()(
						std::vector<std::unique_ptr<ast::block_statement>>{}
					)
				);
			}

			template<typename PoolT>
			std::unique_ptr<ast::class_declaration>
			make_builtin_class_string(PoolT& pool, ast_factory& factory)
			{
				return factory.make<ast::class_declaration>()(
					pool.normalize("java.lang.String"),
					std::vector<std::unique_ptr<ast::var_decl>>{},
					std::vector<std::unique_ptr<ast::instance_method>>{},
					std::vector<std::unique_ptr<ast::main_method>>{}
				);
			}

			template<typename PoolT>
			std::unique_ptr<ast::class_declaration>
			make_builtin_class_printstream(PoolT& pool, ast_factory& factory)
			{
				auto fields = std::vector<std::unique_ptr<ast::var_decl>>{};
				auto methods = std::vector<std::unique_ptr<ast::instance_method>>{};
				methods.push_back(make_builtin_method(
					"println",
					ast::primitive_type::type_void,
					{ast::primitive_type::type_int},
					pool, factory
				));
				methods.push_back(make_builtin_method(
					"write",
					ast::primitive_type::type_void,
					{ast::primitive_type::type_int},
					pool, factory
				));
				methods.push_back(make_builtin_method(
					"flush",
					ast::primitive_type::type_void,
					{},
					pool, factory
				));
				return factory.make<ast::class_declaration>()(
					pool.normalize("java.io.PrintStream"),
					std::move(fields),
					std::move(methods),
					std::vector<std::unique_ptr<ast::main_method>>{}
				);
			}

			template<typename PoolT>
			std::unique_ptr<ast::class_declaration>
			make_builtin_class_inputstream(PoolT& pool, ast_factory& factory)
			{
				auto fields = std::vector<std::unique_ptr<ast::var_decl>>{};
				auto methods = std::vector<std::unique_ptr<ast::instance_method>>{};
				methods.push_back(make_builtin_method(
					"read",
					ast::primitive_type::type_int,
					{},
					pool, factory
				));
				return factory.make<ast::class_declaration>()(
					pool.normalize("java.io.InputStream"),
					std::move(fields),
					std::move(methods),
					std::vector<std::unique_ptr<ast::main_method>>{}
				);
			}

			template<typename PoolT>
			std::unique_ptr<ast::class_declaration>
			make_builtin_class_system(PoolT& pool, ast_factory& factory)
			{
				auto fields = std::vector<std::unique_ptr<ast::var_decl>>{};
				const std::pair<const char*, const char*> members[] = {
					{"in",  "java.io.InputStream"},
					{"out", "java.io.PrintStream"},
				};
				for (auto&& kv : members) {
					fields.push_back(
						factory.make<ast::var_decl>()(
							factory.make<ast::type>()(pool.normalize(kv.second)),
							pool.normalize(kv.first)
						)
					);
				}
				auto methods = std::vector<std::unique_ptr<ast::instance_method>>{};
				methods.push_back(make_builtin_method(
					"id",
					ast::primitive_type::type_int,
					{ast::primitive_type::type_int},
					pool, factory
				));
				methods.push_back(make_builtin_method(
					"exit",
					ast::primitive_type::type_void,
					{ast::primitive_type::type_int},
					pool, factory
				));
				return factory.make<ast::class_declaration>()(
					pool.normalize("java.lang.System"),
					std::move(fields),
					std::move(methods),
					std::vector<std::unique_ptr<ast::main_method>>{}
				);
			}

			template<typename PoolT>
			std::unique_ptr<ast::program>
			make_builtin_ast(PoolT& pool, ast_factory& factory)
			{
				auto classes = std::vector<std::unique_ptr<ast::class_declaration>>{};
				classes.push_back(make_builtin_class_string(pool, factory));
				classes.push_back(make_builtin_class_printstream(pool, factory));
				classes.push_back(make_builtin_class_inputstream(pool, factory));
				classes.push_back(make_builtin_class_system(pool, factory));
				return factory.make<ast::program>()(std::move(classes));
			}

			template<typename PoolT>
			globals_vector make_globals(PoolT& pool, ast_factory& factory)
			{
				auto globals = sem::globals_vector{};
				globals.push_back(factory.make<ast::var_decl>()(
					factory.make<ast::type>()(
						pool.normalize("java.lang.System")
					),
					pool.normalize("System")
				));
				return globals;
			}

		}  // namespace detail

	}  // namespace sem

	template<typename PoolT>
	semantic_info check_program(const ast::program& ast, PoolT& pool, ast_factory& factory)
	{
		// (0) Create the built-in AST.
		auto builtin_ast = sem::detail::make_builtin_ast(pool, factory);
		// (1) Initialize empty annotation and other containers for the
		//     attributes that are relevant for both ASTs.
		auto classes             = sem::class_definitions{};
		auto globals             = sem::globals_vector{};
		auto type_annotations    = sem::type_attributes{};
		auto locals_annotations  = sem::locals_attributes{};
		auto vardecl_annotations = sem::vardecl_attributes{};
		auto method_annotations  = sem::method_attributes{};
		// (2) Define a convenient lambda that captures theses containers and
		//     populates them with the results of a single AST.
		const auto process = [&](const auto& tree, const auto builtin){
			sem::extract_type_info(tree, builtin, classes);
			sem::perform_name_type_analysis(
				tree, !builtin, classes, globals,
				type_annotations, locals_annotations,
				vardecl_annotations, method_annotations
			);
		};
		// (3) Process the built-in AST.  We can (and should) do this before we
		//     have collected the types from the user-provided AST because the
		//     built-in AST had better not refer to anything in the
		//     user-defined one.  If the built-in AST doesn't check in
		//     isolation, there is a bug in the compiler.
		process(*builtin_ast, true);
		// (4) Now we have the built-in types, we can set up the globals for
		//     the user.
		globals = sem::detail::make_globals(pool, factory);
		// (5) And with that, we can also process their AST.
		process(ast, false);
		// (6) We only check return paths and extract constants on the user's
		//     AST.  It wouldn't be wrong to do it for the built-in AST too but
		//     there seems to come no benefit from doing so.
		sem::check_return_paths(ast, type_annotations);
		auto const_annotations = sem::extract_constants(ast);
		// (7) And that's it.  We shall return.
		return semantic_info{
				std::move(classes),
				std::move(type_annotations),
				std::move(locals_annotations),
				std::move(vardecl_annotations),
				std::move(method_annotations),
				std::move(const_annotations),
				std::move(builtin_ast),
				std::move(globals)
		};
	}

}  // namespace minijava
