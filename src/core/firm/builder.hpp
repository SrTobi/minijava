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
#include <exception>

#include "libfirm/firm.h"

#include <parser/ast.hpp>
#include <semantic/semantic.hpp>

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

		struct pair_hash
		{
			std::size_t operator () (const std::pair<ast::type_name, size_t> &p) const {
				size_t seed = 0;
				size_t first_hash = 0;
				auto primitive_type = boost::get<ast::primitive_type>(&p.first);
				if (primitive_type) {
					first_hash = (size_t)*primitive_type;
				} else {
					auto simple_name = boost::get<minijava::symbol>(&p.first);
					first_hash = simple_name->hash();
				}
				boost::hash_combine(seed, first_hash);
				boost::hash_combine(seed, p.second);
				return seed;
			}
		};

		typedef std::pair<ast::type_name, size_t> type_rank_pair;
		typedef std::unordered_map<type_rank_pair, ir_type*, pair_hash> type_mapping;
	}

	class ir_types {
	public:

		ir_types(const semantic_info& info) : _semantic_info{info}
		{
			_type_mapping = firm::type_mapping();
			// collect classes
			for (auto& clazz : info.classes()) {
				create_class_type(clazz);
			}
			// create class structure
			for (auto& clazz : info.classes()) {
				finalize_class_type(clazz);
			}
		}

		// just collect the types for later use
		void create_class_type(const std::pair<const symbol, sem::basic_type_info>& clazz)
		{
			auto class_type = new_type_class(new_id_from_str(clazz.first.data()));
			auto key = std::make_pair(clazz.first, (size_t)0);
			_type_mapping.insert(std::make_pair(key, class_type));
		}

		// add fields and methods
		void finalize_class_type(const std::pair<const symbol, sem::basic_type_info>& clazz)
		{
			assert(clazz.second.is_reference());
			auto class_type = get_class_type(clazz.first);

			// insert fields
			for (auto& field : clazz.second.declaration()->fields()) {
				auto field_var_type = &field->var_type();
				auto ir_type = get_var_type(field_var_type->name(), field_var_type->rank());
				auto field_entity = new_entity(
						class_type,
				        new_id_from_str(field->name().data()),
				        ir_type
				);
				(void)field_entity; // for later use? set_entity_ld_name?
			}

			// insert methods
			for (auto& method : clazz.second.declaration()->instance_methods()) {
				(void)method;
			}
		}

		void create_field_entity()
		{

		}

		ir_type *get_var_type(ast::type_name type, size_t rank = 0) {
			auto key = std::make_pair(type, rank);
			if (_type_mapping.find(key) != _type_mapping.end()) {
				return _type_mapping.at(key);
			}

			// no array
			if (rank == 0) {
				ir_type* simple_type;
				switch (boost::get<ast::primitive_type>(type)) {
					case ast::primitive_type::type_int:
						simple_type = _int_type;
						break;
					case ast::primitive_type::type_boolean:
						simple_type = _boolean_type;
						break;
					case ast::primitive_type::type_void:
						throw std::exception();
					default:
						// at this point, there should already be an type in _type_mapping array
						auto simple_name = boost::get<minijava::symbol>(type);
						simple_type = _type_mapping.at(std::make_pair(simple_name, (size_t)0));
						break;
				}

				return simple_type;
			}

			// handle arrays...
			_type_mapping.insert(std::make_pair(
					key,
					new_type_array(
							get_var_type(type, rank - 1), 0
					)));

			return _type_mapping.at(key);
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
			_int_type = new_type_primitive(_int_mode);
			_boolean_mode = mode_Bs; // maybe use some other type here?
			_boolean_type = new_type_primitive(_boolean_mode);
		}

	private:

		ir_type* get_class_type(symbol name)
		{
			ast::type_name type_name = name;
			return _type_mapping.at(std::make_pair(type_name, (size_t)0));
		}

		firm::type_mapping _type_mapping;

		const semantic_info& _semantic_info;

		ir_mode* _int_mode;
		ir_mode* _boolean_mode;

		ir_type* _int_type;
		ir_type* _boolean_type;
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

		}

	private:
		ir_types _ir_types;
		const semantic_info& _semantic_info;
		const ast::program& _ast;
	};
}
