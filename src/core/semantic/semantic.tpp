#ifndef MINIJAVA_INCLUDED_FROM_SEMANTIC_SEMANTIC_HPP
#error "Never `#include <semantic/semantic.tpp>` directly; `#include <semantic/semantic.hpp>` instead."
#endif

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
				// Currently unused, but let's keep it anyway
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
			globals_vector make_globals(PoolT& pool, ast_factory& factory)
			{
				sem::globals_vector result{};
				result.push_back(factory.make<ast::var_decl>()(
						factory.make<ast::type>()(
								pool.normalize("java.lang.System")
						),
						pool.normalize("System")
				));
				return result;
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
