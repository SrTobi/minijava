/**
 * @file builder.hpp
 *
 * @brief
 *     Creates the irg.
 *
 */

#pragma once

#include <unordered_map>
#include <iostream>
#include <stdio.h>
#include <exception>
#include "exceptions.hpp"

#include "libfirm/firm.h"

#include <parser/ast.hpp>
#include <semantic/semantic.hpp>
#include <semantic/attribute.hpp>
#include "firm/method_builder.hpp"

namespace minijava
{

	namespace firm
	{

		struct sem_type_hash
		{
			std::size_t operator () (const sem::type &p) const {
				size_t seed = 0;
				boost::hash_combine(seed, p.info.declaration());
				boost::hash_combine(seed, p.rank);
				return seed;
			}
		};

		typedef std::unordered_map<sem::type, ir_type*, sem_type_hash> type_mapping;

		typedef ast_attributes<ir_entity*, ast_node_filter<ast::method> > method_mapping;
		typedef ast_attributes<ir_type*, ast_node_filter<ast::class_declaration> > class_mapping;
	}

	class ir_types {
	public:

		ir_types(const semantic_info& info, const ast::program& ast) :
				_semantic_info{info},
		        _ast{ast}
		{
			_type_mapping = firm::type_mapping();
			_method_mapping = firm::method_mapping();
			_class_mapping = firm::class_mapping();
		}

		void init()
		{
			create_basic_types();

			init_types();
			init_methods();

			finalize_class_types();
		}

	private:

		/**
		 * @brief
		 *  collect used ir_types
		 */
		void init_types()
		{
			std::transform(
					std::begin(_semantic_info.type_annotations()),
					std::end(_semantic_info.type_annotations()),
					std::inserter(_type_mapping, _type_mapping.end()),
					[this](auto&& t) {
						return std::make_pair(t.second, this->get_var_type(t.second));
					}
			);
		}

		/**
		 * @brief
		 *  create all class and method ir_types
		 */
		void init_methods()
		{
			for (const auto& clazz : _ast.classes()) {
				// get class type, and maybe create it
				auto class_type = get_class_type(clazz.get());
				// iterate over class methods and create prototypes
				for (auto& method : clazz->instance_methods()) {
					init_method(class_type, *method.get());
				}

				// insert main method
				for (auto& method : clazz->main_methods()) {
					init_method(class_type, *method.get());
				}
			}
		}

		void init_method(ir_type* class_type, ast::instance_method& method)
		{
			auto param_count = method.parameters().size();
			auto return_type = _semantic_info.type_annotations().at(method);
			auto has_return_type = return_type.info.is_void() == false;
			auto method_type = new_type_method(
					param_count + 1, // param count: don't miss implicit this argument
					has_return_type ? 1 : 0, // number of return types
					0, // is variadic?
					cc_cdecl_set, // calling conventions
					mtp_no_property);

			set_method_param_type(method_type, 0, new_type_pointer(class_type));
			size_t param_num = 1;
			for (auto& param : method.parameters()) {
				auto param_type = get_var_type(_semantic_info.type_annotations().at(*param));
				set_method_param_type(method_type, param_num++, param_type);
			}

			if (has_return_type) {
				set_method_res_type(method_type, 0, get_var_type(return_type));
			}

			auto method_entity = new_entity(
					class_type,
					new_id_from_str(method.name().c_str()),
					method_type);
			set_entity_ld_ident(method_entity, new_id_from_str(method.name().c_str()));

			_method_mapping.insert(std::make_pair(&method, method_entity));
		}

		void init_method(ir_type* class_type, ast::main_method& method)
		{
			auto param_count = method.parameters().size();
			auto method_type = new_type_method(
					param_count, // param count: don't miss implicit this argument
					0, // number of return types
					0, // is variadic?
					cc_cdecl_set, // calling conventions
					mtp_no_property);

			auto method_entity = new_entity(
					class_type,
					new_id_from_str(method.name().c_str()),
					method_type);
			set_entity_ld_ident(method_entity, new_id_from_str("mjava_main"));

			_method_mapping.insert(std::make_pair(&method, method_entity));
		}

	public:

		ir_entity* get_method_entity(const ast::method* method)
		{
			if (_method_mapping.find(method) != _method_mapping.end()) {
				return _method_mapping.at(*method);
			}

			MINIJAVA_THROW_ICE(internal_compiler_error);
		}

		ir_type* get_class_type(const sem::type& type)
		{
			return get_class_type(type.info.declaration());
		}

		ir_type* get_class_type(const ast::class_declaration* clazz)
		{
			if (_class_mapping.find(clazz) != _class_mapping.end()) {
				return _class_mapping.at(*clazz);
			}

			return create_class_type(clazz);
		}

	private:

		// just collect the types for later use
		ir_type* create_class_type(const ast::class_declaration* clazz)
		{
			auto type = sem::type(_semantic_info.classes().at(clazz->name()), 0);
			auto class_type = new_type_class(new_id_from_str(clazz->name().data()));
			set_type_alignment(class_type, 8);
			_type_mapping.insert(std::make_pair(type, class_type));
			_class_mapping.insert(std::make_pair(clazz, class_type));
			return class_type;
		}

		void finalize_class_types()
		{
			for (auto& clazz : _ast.classes()) {
				auto type = sem::type(_semantic_info.classes().at(clazz->name()), 0);
				finalize_class_type(type);
			}
		}

		// add fields and methods
		void finalize_class_type(const sem::type& clazz)
		{
			assert(clazz.info.is_reference());
			auto class_type = get_class_type(clazz);

			// insert fields
			int offset = 0;
			for (auto& field : clazz.info.declaration()->fields()) {
				auto field_entity = create_field_entity(class_type, field, offset);
				// todo - better offset calculation
//              size_t type_size = get_type_size(get_entity_type(field_entity));
				(void)field_entity;
				offset += 4;
			}

			// empty entity? insert dummy field to prevent empty entities
			if (offset == 0) {
				auto dummy_field = new_entity(class_type, new_id_from_str("__prevent_empty_class"), _int_type);
				set_entity_offset(dummy_field, 0);
				set_entity_ld_ident(dummy_field, new_id_from_str("__prevent_empty_class"));
			}

			// insert methods
			for (auto& method : clazz.info.declaration()->instance_methods()) {
				create_method_entity(class_type, method);
			}

			for (auto& method : clazz.info.declaration()->main_methods()) {
				create_method_entity(class_type, method);
			}

		}

		ir_entity* create_field_entity(ir_type *class_type, const std::unique_ptr<ast::var_decl> &field, int offset)
		{
			auto field_type = _semantic_info.type_annotations().at(*field);
			auto ir_type = get_var_type(field_type);
			auto field_entity = new_entity(
					class_type,
					new_id_from_str(field->name().data()),
					ir_type
			);
			set_entity_offset(field_entity, offset);
			set_entity_ld_ident(field_entity, new_id_from_str(field->name().data()));
			return field_entity;
		}

		int get_local_var_count(ast::method& node)
		{
			auto locals = _semantic_info.locals_annotations().at(node);
			auto num_locals_ = locals.size();
			auto const max_locals = std::numeric_limits<int>::max();
			// add 1 for "this" parameter
			if (__builtin_add_overflow(num_locals_, 1, &num_locals_) ||
			    num_locals_ > static_cast<std::size_t>(max_locals)) {
				MINIJAVA_THROW_ICE_MSG(
						minijava::internal_compiler_error,
						"Cannot handle functions with more than MAX_INT"
								" local variables"
				);
			}

			auto num_locals = static_cast<int>(num_locals_);
			return num_locals;
		}

		void create_method_entity(ir_type *class_type, const std::unique_ptr<ast::instance_method> &method);
		void create_method_entity(ir_type *class_type, const std::unique_ptr<ast::main_method> &method);

		void create_and_finalize_method_body(const minijava::ast::method &method, ir_graph* irg);

	public:

		ir_type *get_var_type(sem::type type)
		{
			if (_type_mapping.find(type) != _type_mapping.end()) {
				return _type_mapping.at(type);
			}

			// no array
			if (type.rank == 0) {
				ir_type* simple_type;
				if (type.info.is_boolean()) {
					simple_type = _boolean_type;
				} else if (type.info.is_int()) {
					simple_type = _int_type;
				} else if (type.info.is_void()) {
					simple_type = _void_type;
				} else if (type.info.is_reference()) {
					simple_type = get_class_type(type);
				}

				return simple_type;
			}

			// handle arrays...
			auto recursive_type = new_type_array(
					get_var_type(sem::type{type.info, type.rank - 1}), 0
			);
			_type_mapping.insert(std::make_pair(
					type,
					recursive_type));

			return recursive_type;
		}

		ir_type* type_boolean() const noexcept
		{
			return _boolean_type;
		}

		ir_type* type_int() const noexcept
		{
			return _int_type;
		}

		ir_mode* mode_boolean() const noexcept {
			return _boolean_mode;
		}

		ir_mode* mode_int() const noexcept
		{
			return _int_mode;
		}

		void create_entities()
		{

		}

		void create_basic_types()
		{
			_int_mode = mode_Is;
			_boolean_mode = new_int_mode("B", irma_twos_complement, 8, 0, 1);

			_int_type = new_type_primitive(_int_mode);
			_boolean_type = new_type_primitive(_boolean_mode);
		}

	private:

		firm::type_mapping _type_mapping;
		firm::method_mapping _method_mapping;
		firm::class_mapping _class_mapping;

		const semantic_info& _semantic_info;
		const ast::program& _ast;

		ir_mode* _int_mode;
		ir_mode* _boolean_mode;
		ir_mode* _void_mode; // just for convenience

		ir_type* _int_type;
		ir_type* _boolean_type;
		ir_type* _void_type;
	};

	class builder
	{

	public:

		builder(const ast::program& ast, const semantic_info& semantic_info) :
				_ir_types{ir_types(semantic_info, ast)},
				_semantic_info{semantic_info},
		        _ast{ast}
		{
			ir_init();
			// turn off optimizations
			set_optimize(0);
			// set pointer mode
			auto mode_p = new_reference_mode("P64", irma_twos_complement, 64, 64);
			set_modeP(mode_p);
		}

		~builder()
		{
			ir_finish();
		}

		void ast2firm()
		{
			_ir_types.init();
		}

		void emit()
		{
			be_parse_arg("isa=amd64");
			auto f = std::fopen("./temp.asm", "w+");
			if (f != NULL) {
				be_main(f, "main_class");
				std::fclose(f);
			}
		}

		void dump_graph(ir_graph* irg, std::string suffix)
		{
			dump_ir_graph(irg, suffix.c_str());
		}

	private:

		ir_types _ir_types;
		const semantic_info& _semantic_info;
		const ast::program& _ast;
	};
}
