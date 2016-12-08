#include "firm/type_builder.hpp"

namespace minijava
{

	namespace firm
	{

		namespace /* anonymous */
		{

			class ir_types_impl
			{
			public:

				ir_types_impl(const ast::program& ast, const semantic_info& seminfo)
					: _ast{ast}
					, _seminfo{seminfo}
					, _primitives{primitive_types::get_instance()}
				{
					_init_types();
					_init_methods();
					_finalize_class_types();
				}

				ir_types get() &&
				{
					return ir_types{
						std::move(_typemap),
						std::move(_classmap),
						std::move(_fieldmap),
						std::move(_methodmap)
					};
				}

			private:

				const ast::program& _ast;
				const semantic_info& _seminfo;
				primitive_types _primitives{};

				type_mapping _typemap{};
				class_mapping _classmap{};
				field_mapping _fieldmap{};
				method_mapping _methodmap{};

				void _init_types()
				{
					for (const auto& kv : _seminfo.type_annotations()) {
						if (!kv.second.info.is_void()) {
							_typemap.insert({kv.second, _get_var_type(kv.second)});
						}
					}
				}

				ir_type* _get_var_type(const sem::type type)
				{
					assert(!type.info.is_void());
					const auto pos = _typemap.find(type);
					if (pos != _typemap.end()) {
						return pos->second;
					}
					if (type.rank == 0) {
						if (type.info.is_boolean()) {
							return _primitives.boolean_type;
						} else if (type.info.is_int()) {
							return _primitives.int_type;
						} else if (type.info.is_reference()) {
							return _create_class_type(*type.info.declaration()).second;
						} else {
							MINIJAVA_NOT_REACHED();
						}
					}
					// TODO: Use iteration instead of recursion
					auto recursive_type = new_type_array(
						_get_var_type(sem::type{type.info, type.rank - 1}),
						0
					);
					_typemap.insert({type, recursive_type});
					return recursive_type;
				}

				ir_type* _get_class_type(const ast::class_declaration& clazz)
				{
					const auto pos = _classmap.find(&clazz);
					if (pos != _classmap.end()) {
						return pos->second;
					}
					return _create_class_type(clazz).first;
				}

				void _init_methods()
				{
					for (const auto& clazz : _ast.classes()) {
						const auto class_type = _get_class_type(*clazz.get());
						for (const auto& method : clazz->instance_methods()) {
							_init_method(class_type, *method);
						}
						for (const auto& method : clazz->main_methods()) {
							_init_method(get_glob_type(), *method);
						}
					}
				}

				void _init_method(ir_type* class_type, const ast::instance_method& method)
				{
					const auto param_count = method.parameters().size();
					const auto return_type = _seminfo.type_annotations().at(method);
					const auto has_return_type = !return_type.info.is_void();
					const auto method_type = new_type_method(
						param_count + 1,         // param count (+1 for this)
						has_return_type ? 1 : 0, // number of return types
						0,                       // variadic?
						cc_cdecl_set,            // calling convention
						mtp_no_property
					);
					set_method_param_type(method_type, 0, new_type_pointer(class_type));
					auto param_num = std::size_t{1};
					for (const auto& param : method.parameters()) {
						const auto param_type = _get_var_type(_seminfo.type_annotations().at(*param));
						set_method_param_type(method_type, param_num++, param_type);
					}
					if (has_return_type) {
						set_method_res_type(method_type, 0, _get_var_type(return_type));
					}
					const auto method_entity = new_entity(
						class_type,
						new_id_from_str(method.name().c_str()),
						method_type);
					set_entity_ld_ident(method_entity, new_id_from_str(method.name().c_str()));  // TODO: mangle
					_methodmap.insert({&method, method_entity});
				}

				void _init_method(ir_type* class_type, const ast::main_method& method)
				{
					const auto method_type = new_type_method(
						0,               // param count
						0,               // number of return types
						0,               // variadic?
						cc_cdecl_set,    // calling convention
						mtp_no_property
					);
					const auto minijava_main = new_id_from_str("minijava_main");
					const auto method_entity = new_entity(
						class_type,
						minijava_main,
						method_type
					);
					set_entity_ld_ident(method_entity, minijava_main);
					_methodmap[method] = method_entity;
				}

				std::pair<ir_type*, ir_type*> _create_class_type(const ast::class_declaration& clazz)
				{
					const auto type = sem::type{_seminfo.classes().at(clazz.name()), 0};
					const auto class_type = new_type_class(new_id_from_str(clazz.name().c_str()));
					const auto pointer_type = new_type_pointer(class_type);
					set_type_alignment(class_type, 8);
					_typemap[type] = pointer_type;
					_classmap[clazz] = class_type;
					return std::make_pair(class_type, pointer_type);
				}

				void _finalize_class_types()
				{
					for (const auto& clazz : _ast.classes()) {
						const auto type = sem::type{_seminfo.classes().at(clazz->name()), 0};
						_finalize_class_type(type);
					}
				}

				// add fields and methods
				void _finalize_class_type(const sem::type clazz)
				{
					assert(clazz.info.is_reference());
					const auto class_type = _classmap.at(*clazz.info.declaration());
					// insert fields
					for (const auto& field : clazz.info.declaration()->fields()) {
						_create_field_entity(class_type, *field);
					}
					// TODO: Is there a better way to trick Firm into accepting empty types?
					if (clazz.info.declaration()->fields().empty()) {
						const auto dummy_name = new_id_from_str("__prevent_empty_class");
						const auto dummy_field = new_entity(class_type, dummy_name, _primitives.int_type);
						set_entity_ld_ident(dummy_field, dummy_name);
					}

					default_layout_compound_type(class_type);
				}

				ir_entity* _create_field_entity(ir_type *class_type, const ast::var_decl& field)
				{
					const auto field_type = _seminfo.type_annotations().at(field);
					const auto ir_type = _get_var_type(field_type);
					const auto field_entity = new_entity(
						class_type,
						new_id_from_str(field.name().c_str()),
						ir_type
					);
					set_entity_ld_ident(field_entity, new_id_from_str(field.name().c_str()));  // TODO: mangle
					_fieldmap[field] = field_entity;
					return field_entity;
				}


			};  // class ir_types_impl

		}  // namespace /* anonymous */


		const primitive_types& primitive_types::get_instance()
		{
			static const auto instance = [](){
				auto pt = primitive_types{};
				pt.int_mode = mode_Is;
				pt.boolean_mode = new_int_mode("B", irma_twos_complement, 8, 0, 1);
				pt.int_type = new_type_primitive(pt.int_mode);
				pt.boolean_type = new_type_primitive(pt.boolean_mode);
				pt.pointer_mode = mode_P;
				pt.pointer_type = new_type_primitive(mode_P);
				return pt;
			}();
			return instance;
		}


		const runtime_library& runtime_library::get_instance()
		{
			static const auto instance = [](){
				auto primitives = primitive_types::get_instance();
				auto rt = runtime_library{};
				// create allocate method
				rt.alloc_type = new_type_method(2, 1, 0, cc_cdecl_set, mtp_no_property);
				set_method_param_type(rt.alloc_type, 0, primitives.int_type);
				set_method_param_type(rt.alloc_type, 1, primitives.int_type);
				set_method_res_type(rt.alloc_type, 0, primitives.pointer_type);
				rt.alloc = new_entity(get_glob_type(), new_id_from_str("mj_runtime_allocate"), rt.alloc_type);
				// create println method
				rt.println_type = new_type_method(1, 0, 0, cc_cdecl_set, mtp_no_property);
				set_method_param_type(rt.println_type, 0, primitives.int_type);
				rt.println = new_entity(get_glob_type(), new_id_from_str("mj_runtime_println"), rt.println_type);
				return rt;
			}();
			return instance;
		}


		ir_types create_types(const ast::program& ast, const semantic_info& seminfo)
		{
			auto result = ir_types_impl{ast, seminfo};
			return std::move(result).get();
		}

	}  // namespace firm

}  // namespace minijava
