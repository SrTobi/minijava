#include "firm/mangle.hpp"


namespace minijava
{

	namespace firm
	{

		ident* mangle(firm_ir& /* firm */, const symbol classname, const symbol membername)
		{
			return new_id_fmt("%s.%s", classname.c_str(), membername.c_str());
		}

	}  // namespace firm

}  // namespace minijava
