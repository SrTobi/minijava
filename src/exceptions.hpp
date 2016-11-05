/**
 * @file exceptions.hpp
 *
 * @brief
 *     Generic project-wide exceptions.
 *
 */


#pragma once

#include <stdexcept>
#include <string>


/**
 * @brief
 *     Convenience macro to create and immediately `throw` an
 *     `internal_compiler_error` with a message identifying the current source
 *     code location.
 *
 * @param IceT
 *     exception type derived from `internal_compiler_error` to `throw`
 *
 */
#define MINIJAVA_THROW_ICE(IceT)                                              \
	::minijava::throw_ice<IceT>(__FILE__, __LINE__, __func__)


/**
 * @brief
 *     Convenience macro to create an `internal_compiler_error` with a message
 *     identifying the current source code location.
 *
 * @param IceT
 *     exception type derived from `internal_compiler_error` to `throw`
 *
 * @param Msg
 *     additional custom error message (string)
 *
 */
#define MINIJAVA_THROW_ICE_MSG(IceT, Msg)                                     \
	::minijava::throw_ice<IceT>(__FILE__, __LINE__, __func__, Msg)


/**
 * @brief
 *     Use this macro to annotate places in the code that -- by your honest
 *     reasoning -- can never be reached unless your code has a bug.
 *
 * The current implementation of this macro `throw`s an ICE but this is subject
 * to change.  The only thing you can rely on is that the macro will always
 * have the effect of interrupting the linear control flow; possibly by
 * terminating the application.
 *
 */
#define MINIJAVA_NOT_REACHED()                                                \
	MINIJAVA_THROW_ICE_MSG(                                                   \
		::minijava::internal_compiler_error,                                  \
		"The impossible has happened"                                         \
	)


namespace minijava
{

	/**
	 * @brief
	 *     Generic exception type for reporting internal compiler errors.
	 *
	 * This exception should only be `throw`n in situations that are
	 * non-recoverable.
	 *
	 */
	struct internal_compiler_error : std::runtime_error
	{

		/**
		 * @brief
		 *     Creates an exception object with a generic error message.
		 *
		 */
		internal_compiler_error();

		/**
		 * @brief
		 *     Creates an exception object with the provided error message.
		 *
		 * @param msg
		 *     error message
		 *
		 */
		explicit internal_compiler_error(const std::string& msg);

		/**
		 * @brief
		 *     Creates an exception object with the provided source code
		 *     location information.
		 *
		 * @param file
		 *     name of the source file
		 *
		 * @param line
		 *     source code line number
		 *
		 * @param function
		 *     name of the containing function
		 *
		 */
		internal_compiler_error(const std::string& file,
		                        int line,
		                        const std::string& function);

		/**
		 * @brief
		 *     Creates an exception object with the provided source code
		 *     location information and a custom error message.
		 *
		 * @param file
		 *     name of the source file
		 *
		 * @param line
		 *     source code line number
		 *
		 * @param function
		 *     name of the containing function
		 *
		 * @param msg
		 *     error message
		 *
		 */
		internal_compiler_error(const std::string& file,
		                        int line,
		                        const std::string& function,
		                        const std::string& msg);

	};


	/**
	 * @brief
	 *     Exception type for reporting errors due to not yet implemented
	 *     functionality.
	 *
	 * This `class` inherits all constructors of `internal_compiler_error`.
	 *
	 */
	struct not_implemented_error : internal_compiler_error
	{

		using internal_compiler_error::internal_compiler_error;

	};


	/**
	 * @brief
	 *     Helper function to `throw` an ICE.
	 *
	 * @tparam IceT
	 *     exception type derived from `internal_compiler_error` to `throw`
	 *
	 * @param file
	 *     name of the source file
	 *
	 * @param line
	 *     source code line number
	 *
	 * @param function
	 *     name of the containing function
	 *
	 */
	template <typename IceT>
	[[noreturn]]
	std::enable_if_t<std::is_base_of<internal_compiler_error, IceT>{}>
	throw_ice(const std::string& file, const int line,
	          const std::string& function)
	{
		throw IceT{file, line, function};
	}


	/**
	 * @brief
	 *     Helper function to `throw` an ICE.
	 *
	 * @tparam IceT
	 *     exception type derived from `internal_compiler_error` to `throw`
	 *
	 * @param file
	 *     name of the source file
	 *
	 * @param line
	 *     source code line number
	 *
	 * @param function
	 *     name of the containing function
	 *
	 * @param msg
	 *     error message
	 *
	 */
	template <typename IceT>
	[[noreturn]]
	std::enable_if_t<std::is_base_of<internal_compiler_error, IceT>{}>
	throw_ice(const std::string& file, const int line,
	          const std::string& function, const std::string& msg)
	{
		throw IceT{file, line, function, msg};
	}

}  // namespace minijava
