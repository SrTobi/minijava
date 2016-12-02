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

#include "libfirm/firm.h"

#include <parser/ast.hpp>
#include <semantic/semantic.hpp>
#include <semantic/attribute.hpp>

namespace minijava
{

	namespace firm
	{

		/*
		struct collect_methods_visitor final : ast::visitor
		{

			collect_methods_visitor(ir_types ir_types)
			{
				_ir_types = irtypes;
			}

			void visit(const ast::instance_method& mthd) override
			{
				auto num_of_local_vars = 9999; // TODO
				auto method_type = new_type_method(mthd.parameters().size(), 1);
				// set params for method
				size_t pos = 0;
				for (auto param : mthd.parameters()) {
					auto param = mthd.parameters().at(i);
					auto param_type = _ir_types.get_var_type(param->var_type());
					set_method_param_type(method_type, pos++, param_type);
				}
				// set return type
				set_method_res_type(method_type, 0, _ir_types.get_var_type(mthd.return_type()));
				// create entity
				auto method_entity = new_entity(get_glob_type(), new_id_from_str(mthd.name().data()), method_type);
				// create graph
				auto fun_graph = new_ir_graph(method_type, num_of_local_vars);
				set_current_ir_graph(fun_graph);
				// todo - create graph nodes...
				irg_finalize_cons(fun_graph);
			}

		private:

			ir_types _ir_types;
		}*/

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

		typedef ast_attributes<ir_type*, ast_node_filter<ast::method> > method_mapping;
		typedef ast_attributes<ir_type*, ast_node_filter<ast::class_declaration> > class_mapping;
	}

	class ir_types {
	public:

		ir_types(const semantic_info& info) :
				_semantic_info{info}
		{
			_type_mapping = firm::type_mapping();
			_method_mapping = firm::method_mapping();
			_class_mapping = firm::class_mapping();
		}

		void init()
		{
			// collect used ir_types
			std::transform(
					std::begin(_semantic_info.type_annotations()),
					std::end(_semantic_info.type_annotations()),
					std::inserter(_type_mapping, _type_mapping.end()),
					[this](auto&& t) {
						return std::make_pair(t.second, this->get_var_type(t.second));
					}
			);
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
			_type_mapping.insert(std::make_pair(type, class_type));
			_class_mapping.insert(std::make_pair(clazz, class_type));
			return class_type;
		}

	public:

		// add fields and methods
		void finalize_class_type(const sem::type& clazz)
		{
			assert(clazz.info.is_reference());
			auto class_type = get_class_type(clazz);

			// insert fields
			for (auto& field : clazz.info.declaration()->fields()) {
				create_field_entity(class_type, field);
			}

			// insert methods
			for (auto& method : clazz.info.declaration()->instance_methods()) {
				create_method_entity(class_type, method);
			}
		}

		void create_field_entity(ir_type *class_type, const std::unique_ptr<ast::var_decl> &field)
		{
			auto field_type = _semantic_info.type_annotations().at(*field);
			auto ir_type = get_var_type(field_type);
			auto field_entity = new_entity(
					class_type,
					new_id_from_str(field->name().data()),
					ir_type
			);
			(void)field_entity; // for later use? set_entity_ld_name?
		}

		void create_method_entity(ir_type *class_type, const std::unique_ptr<ast::instance_method> &method);

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
				_ir_types{ir_types(semantic_info)},
				_semantic_info{semantic_info},
		        _ast{ast}
		{
			ir_init();
		}

		~builder()
		{
			ir_finish();
		}

		void ast2firm()
		{
			_ir_types.init();

			for (const auto& clazz : _ast.classes()) {
				for (auto& method : clazz->instance_methods()) {
					auto param_count = method->parameters().size() + 1; // don't miss implizit this argument
					auto return_type = _semantic_info.type_annotations().at(*method);
					auto ir_type = new_type_method(
							param_count, // param count
							return_type.info.is_void() ? 0 : 1, // number of return types
							0, // is variadic?
							cc_cdecl_set, // calling conventions
							mtp_no_property);
					(void)ir_type;
				}


			}
		}

		void emit()
		{
//          be_parse_arg("isa=amd64");
//          auto f = std::fopen("temp.asm", "w+");
//          if (f != NULL) {
//              be_main(f, "main_class");
//              std::fclose(f);
//          }
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
