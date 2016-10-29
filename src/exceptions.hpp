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
 *     Convenience macro to create an `internal_compiler_error` with a message
 *     identifying the current source code location.
 *
 */
#define MINIJAVA_MAKE_ICE()  \
	::minijava::internal_compiler_error{ __FILE__, __LINE__, __func__ }


/**
 * @brief
 *     Convenience macro to create an `internal_compiler_error` with a message
 *     identifying the current source code location.
 *
 * @param Msg
 *     additional custom error message (string)
 *
 */
#define MINIJAVA_MAKE_ICE_MSG(Msg)  \
	::minijava::internal_compiler_error{ __FILE__, __LINE__, __func__, Msg }


// TBD: The two macros defined above only uses standard C++ features.  If we
//      could agree to use GCC's `__builtin_FILE` and other magic functions,
//      then the macros wouldn't be needed and could be implemented as ordinary
//      functions instead.


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
	 */
	struct not_implemented_error : internal_compiler_error
	{

		/**
		 * @brief
		 *     Creates an exception object.
		 *
		 */
		not_implemented_error();

	};


}  // namespace minijava
