#include <iostream>
#include <assert.h>

#include "irg/lowering.hpp"
#include "libfirm/firm.h"

namespace minijava
{

	namespace
	{

		void layout_method(ir_type * /*clazz*/, ir_entity *method)
		{
			assert(is_method_entity(method));
			// move method to global type
			set_entity_owner(method, get_glob_type());
		}

		// move all class methods to global type
		void layout_class(ir_type *type)
		{
			assert(is_Class_type(type));
			auto member_size = get_class_n_members(type);
			for (size_t j = member_size; j > 0; j--) {
				auto member = get_class_member(type, j - 1);
				if (is_method_entity(member)) {
					layout_method(type, member);
				}
			}
			set_type_state(type, layout_fixed);
		}

		void layout_types()
		{
			auto num_types = get_irp_n_types();
			auto glob = get_glob_type();
			for (size_t i = 0; i < num_types; i++) {
				auto type = get_irp_type(i);
				if (type == glob) {
					continue;
				}

				if (is_Class_type(type) && !is_frame_type(type)) {
					layout_class(type);
				}
			}
		}
	}

	void irg::lower() {
		// layout all types for later use
		layout_types();
		// replaces Offsets, TypeConsts by real constants(if possible)
		// replaces Members and Sel nodes by address computation
		lower_highlevel();
	}
}
