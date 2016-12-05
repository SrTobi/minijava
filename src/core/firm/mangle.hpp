/**
 * @file mangle.hpp
 *
 * @brief
 *     Name mangling for Firm.
 *
 */

#pragma once

#include <string>

#include "firm/firm.hpp"
#include "symbol/symbol.hpp"

#include "libfirm/ident.h"


namespace minijava
{

	namespace firm
	{

		/**
		 * @brief
		 *     Constructs a mangled Firm identifier.
		 *
		 * @param firm
		 *     handle for current Firm session
		 *
		 * @param classname
		 *     name of the class
		 *
		 * @param membername
		 *     name of the class memeber (field or method)
		 *
		 * @returns
		 *     handle to Firm `ident` of mangled name
		 *
		 */
		ident* mangle(firm_ir& firm, symbol classname, symbol membername);

	}  // namespace firm

}  // namespace minijava
