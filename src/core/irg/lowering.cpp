#include <iostream>
#include <assert.h>

#include "irg/lowering.hpp"
#include "firm.hpp"

namespace minijava
{

	namespace
	{

		void layout_method(firm::ir_type* /*clazz*/, firm::ir_entity* method)
		{
			assert(firm::is_method_entity(method));
			// move method to global type
			firm::set_entity_owner(method, firm::get_glob_type());
		}

		// move all class methods to global type
		void layout_class(firm::ir_type* type)
		{
			assert(is_Class_type(type));
			auto member_size = firm::get_class_n_members(type);
			for (size_t j = member_size; j > 0; j--) {
				auto member = get_class_member(type, j - 1);
				if (firm::is_method_entity(member)) {
					layout_method(type, member);
				}
			}
			set_type_state(type, firm::layout_fixed);
		}

		void layout_types()
		{
			auto num_types = firm::get_irp_n_types();
			auto glob = firm::get_glob_type();
			for (size_t i = 0; i < num_types; i++) {
				auto type = firm::get_irp_type(i);
				if (type == glob) {
					continue;
				}

				if (firm::is_Class_type(type) && !firm::is_frame_type(type)) {
					layout_class(type);
				}
			}
		}
	}

	void layout_graphs()
	{
		auto num_graphs = firm::get_irp_n_irgs();
		for (size_t i = 0; i < num_graphs; i++) {
			auto irg = firm::get_irp_irg(i);
			firm::lower_mux(irg, nullptr);
		}
	}

	void irg::lower() {
		// layout all types for later use
		layout_types();
		// layout graphs
		layout_graphs();
		// replaces Offsets, TypeConsts by real constants(if possible)
		// replaces Members and Sel nodes by address computation
		firm::lower_highlevel();
	}
}
