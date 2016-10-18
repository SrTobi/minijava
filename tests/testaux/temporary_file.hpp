/**
 * @file temporary_file.hpp
 *
 * @brief
 *         Ephermal file system entries.
 *
 */

#pragma once

#include <string>


namespace testaux
{

	/**
	 * @brief
	 *         A temporary file that is opened in the constructor and
	 *         deleted in the destructor.
	 *
	 * This feature is by no means secure.  Even a moderately
	 * sophisticated attacker will be able to provoke reace conditions
	 * on the generated file names.
	 *
	 */
	class temporary_file final
	{
	private:

		/** @brief Name of the temporary file. */
		std::string _filename {};

	public:

		/**
		 * @brief
		 *         Creates a temporary file with the given contents.
		 *
		 * @param text
		 *         initial contents of the file
		 *
		 * @throws std::system_error
		 *         upon failure to create the file
		 *
		 */
		temporary_file(const std::string& text = "");

		/** @brief Deletes the temporary file. */
		~temporary_file();

		/**
		 * @brief
		 *         `return`s the filename of the temporary file.
		 *
		 * @returns
		 *         filename of the temporary file
		 *
		 */
		std::string filename() const
		{
			return _filename;
		}

	};  // class temporary_file

}  // namespace testaux
