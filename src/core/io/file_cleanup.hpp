/**
 * @file file_cleanup.hpp
 *
 * @brief
 *     Removal of temporary files.
 *
 */

#pragma once

#include <string>


namespace minijava
{

	/**
	 * @brief
	 *     RAII guard for reliable removal of temporary files.
	 *
	 * The files are not created by this class.  Its destructor will remove
	 * them, but silently skip non-existing files.  If the environment variable
	 * `MINIJAVA_KEEP_TEMPORARY_FILES` is set to a non-empty string, no files
	 * will be deleted.
	 *
	 */
	class file_cleanup final
	{
	public:

		/**
		 * @brief
		 *     Creates a guard object that will delete the named file in its
		 *     destructor.
		 *
		 * @param filename
		 *     name of the file to delete
		 *
		 */
		file_cleanup(std::string filename);

		/**
		 * `delete`d copy constructor.
		 *
		 * @param other
		 *     *N/A*
		 *
		 */
		file_cleanup(const file_cleanup& other) = delete;

		/**
		 * `delete`d copy-assignment operator.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		file_cleanup& operator=(const file_cleanup& other) = delete;

		/**
		 * @brief
		 *     Deletes the guarded file.
		 *
		 * If the file does not exist (any more) nothing is done.  If there is
		 * an error, it is printed to standard error output and otherwise
		 * ignored.
		 *
		 */
		~file_cleanup();

	private:

		/** @brief Name of the to-be-deleted file. */
		std::string _filename{};
	};

}
