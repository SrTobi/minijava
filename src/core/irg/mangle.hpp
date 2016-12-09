/**
 * @file mangle.hpp
 *
 * @brief
 *     Name mangling for Firm.
 *
 */

#pragma once

#include <string>

#include "irg/irg.hpp"
#include "parser/ast.hpp"

#include "libfirm/ident.h"


namespace minijava
{

	namespace irg
	{

		/**
		 * @brief
		 *     Constructs a mangled Firm identifier for a class name.
		 *
		 * If `class.name()` is not a valid assembly identifier, the behavior
		 * is undefined.
		 *
		 * @param clazz
		 *     class
		 *
		 * @returns
		 *     handle to Firm `ident` of mangled name
		 *
		 */
		ident* mangle(const ast::class_declaration& clazz);

		/**
		 * @brief
		 *     Constructs a mangled Firm identifier for a (global) variable
		 *     name.
		 *
		 * If `vardecl.name()` is not a valid assembly identifier, the behavior
		 * is undefined.
		 *
		 * @param vardecl
		 *     variable declaration
		 *
		 * @returns
		 *     handle to Firm `ident` of mangled name
		 *
		 */
		ident* mangle(const ast::var_decl& vardecl);

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
		ident* mangle(const ast::class_declaration& clazz, const ast::var_decl& field);

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
		ident* mangle(const ast::class_declaration& clazz, const ast::instance_method& method);

	}  // namespace irg

}  // namespace minijava
