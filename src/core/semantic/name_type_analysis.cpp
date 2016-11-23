#include "semantic/name_type_analysis.hpp"

#include "exceptions.hpp"


namespace minijava
{
	namespace sem
	{
		void perform_shallow_type_analysis(const ast::program&  /* ast              */,
										   type_attributes&     /* type_annotations */)
		{
			MINIJAVA_THROW_ICE(not_implemented_error);
		}

		void perform_full_name_type_analysis(const ast::program& /* ast                */,
											 const globals_map&  /* globals             */,
											 type_attributes&    /* type_annotations    */,
											 locals_attributes&  /* locals_annotations  */,
											 vardecl_attributes& /* vardecl_annotations */,
											 method_attributes&  /* method_annotations  */)
		{
			MINIJAVA_THROW_ICE(not_implemented_error);
		}

	}  // namespace sem

}  // namespace minijava
