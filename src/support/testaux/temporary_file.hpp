/**
 * @file temporary_file.hpp
 *
 * @brief
 *     Ephermal file system entries and related utility functions.
 *
 */

#pragma once

#include <cstdio>
#include <memory>
#include <string>


namespace testaux
{

	/**
	 * @brief
	 *     A temporary file that is opened in the constructor and deleted in
	 *     the destructor.
	 *
	 * This feature is by no means secure.  Even a moderately sophisticated
	 * attacker will be able to provoke reace conditions on the generated file
	 * names.
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
		 *     Creates a temporary file with the given contents.
		 *
		 * @param text
		 *     initial contents of the file
		 *
		 * @param suffix
		 *     optional file name suffix
		 *
		 * @throws std::system_error
		 *     upon failure to create the file
		 *
		 */
		temporary_file(const std::string& text = "", const std::string& suffix = "");

		/** @brief Deletes the temporary file. */
		~temporary_file();

		/**
		 * @brief
		 *     `delete`d copy constructor.
		 *
		 * `temporary_file`s are not copyable and not moveable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 */
		temporary_file(const temporary_file& other) = delete;

		/**
		 * @brief
		 *     `delete`d copy-assignment operator.
		 *
		 * `temporary_file`s are not copyable and not moveable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		temporary_file& operator=(const temporary_file& other) = delete;

		/**
		 * @brief
		 *     `return`s the filename of the temporary file.
		 *
		 * @returns
		 *     filename of the temporary file
		 *
		 */
		const std::string& filename() const noexcept
		{
			return _filename;
		}

	};  // class temporary_file


	/**
	 * @brief
	 *     A temporary directory that is created in the constructor and deleted
	 *     recursively in the destructor.
	 *
	 * This feature is by no means secure.  Even a moderately sophisticated
	 * attacker will be able to provoke reace conditions on the generated file
	 * names.
	 *
	 */
	class temporary_directory final
	{
	private:

		/** @brief Absolute file name of the temporary directory. */
		std::string _filename {};

	public:

		/**
		 * @brief
		 *     Creates an empty temporary directory.
		 *
		 * @throws std::system_error
		 *     upon failure to create the directory
		 *
		 */
		temporary_directory();

		/** @brief Deletes the temporary directory recursively. */
		~temporary_directory();

		/**
		 * @brief
		 *     `delete`d copy constructor.
		 *
		 * `temporary_directory`s are not copyable and not moveable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 */
		temporary_directory(const temporary_directory& other) = delete;

		/**
		 * @brief
		 *     `delete`d copy-assignment operator.
		 *
		 * `temporary_directory`s are not copyable and not moveable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		temporary_directory& operator=(const temporary_directory& other) = delete;

		/**
		 * @brief
		 *     `return`s the absolute path of the temporary directory.
		 *
		 * @returns
		 *     filename of the temporary directory
		 *
		 */
		const std::string& filename() const noexcept
		{
			return _filename;
		}

		/**
		 * @brief
		 *     `return`s the absolute path of the file `local` relative to the
		 *     temporary directory.
		 *
		 * For example, if the temporary directory is `/tmp/1492/` then
		 * `filename("file.txt")` will give `/tmp/1492/file.txt`.
		 *
		 * @returns
		 *     filename of the file inside the temporary directory
		 *
		 */
		std::string filename(const std::string& local) const;

	};  // class temporary_directory


	/**
	 * @brief
	 *     Tests whether the file `filename` has the `expected` content.
	 *
	 * @param filename
	 *     file-name of the file to read
	 *
	 * @param expected
	 *     data to compare the file's data with
	 *
	 * @returns
	 *     `true` if and only if the file can be read and has the expected content
	 *
	 * @throws std::ios_base::failure
	 *     if the file cannot be read
	 *
	 */
	bool file_has_content(const std::string& filename, const std::string& expected);

	/**
	 * @brief
	 *     Opens a file and `return`s an RAII wrapper of the `FILE` pointer.
	 *
	 * @param filename
	 *     file-name of the file to open
	 *
	 * @param mode
	 *     mode in which to open the file (`[arw]b?`)
	 *
	 * @returns
	 *     `std::unique_ptr` holding the open file handle
	 *
	 * @throws std::system_error
	 *     if the file cannot be opened
	 *
	 */
	std::unique_ptr<std::FILE, decltype(&std::fclose)>
	open_file(const std::string& filename, const std::string& mode);

}  // namespace testaux
