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
#include "parser/ast.hpp"

#include "libfirm/ident.h"


namespace minijava
{

	namespace firm
	{

		/**
		 * @brief
		 *     Constructs a mangled Firm identifier for a class name.
		 *
		 * If `class.name()` is not a valid assembly identifier, the behavior
		 * is undefined.
		 *
		 * @param firm
		 *     handle for current Firm session
		 *
		 * @param clazz
		 *     class
		 *
		 * @returns
		 *     handle to Firm `ident` of mangled name
		 *
		 */
		ident* mangle(firm_ir& firm, const ast::class_declaration& clazz);

		/**
		 * @brief
		 *     Constructs a mangled Firm identifier for a field.
		 *
		 * If `class.name()` or `field.name()` is not a valid assembly
		 * identifier, the behavior is undefined.
		 *
		 * If `field` is not actually a field of `clazz`, the behavior is
		 * undefined.
		 *
		 * @param firm
		 *     handle for current Firm session
		 *
		 * @param clazz
		 *     class containing the field
		 *
		 * @param field
		 *     field contained in the class
		 *
		 * @returns
		 *     handle to Firm `ident` of mangled name
		 *
		 */
		ident* mangle(firm_ir& firm, const ast::class_declaration& clazz, const ast::var_decl& field);

		/**
		 * @brief
		 *     Constructs a mangled Firm identifier for a method.
		 *
		 * If `class.name()` or `method.name()` is not a valid assembly
		 * identifier, the behavior is undefined.
		 *
		 * If `method` is not actually a methos of `clazz`, the behavior is
		 * undefined.
		 *
		 * @param firm
		 *     handle for current Firm session
		 *
		 * @param clazz
		 *     class containing the method
		 *
		 * @param method
		 *     method contained in the class
		 *
		 * @returns
		 *     handle to Firm `ident` of mangled name
		 *
		 */
		ident* mangle(firm_ir& firm, const ast::class_declaration& clazz, const ast::instance_method& method);

	}  // namespace firm

}  // namespace minijava
