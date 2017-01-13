/**
 * @file file_output.hpp
 *
 * @brief
 *     A simple fast RAII wrapper around C-style file output with correct error
 *     handling.
 *
 */

#pragma once

#include <cstdio>
#include <memory>
#include <string>


namespace minijava
{

	/**
	 * @brief
	 *     An RAII wrapper around a `FILE` pointer with correct error handling.
	 *
	 * This `class` is a RAII wrapper around a `FILE` pointer.  The provided
	 * member functions do little more than forwarding to the corresponding C
	 * functions plus additional error checking.  It uses exceptions for error
	 * handling.  If any operation fails, a `std::system_error` is `throw`n.
	 *
	 * A `file_output` object can be in three states:
	 *
	 *  - empty,
	 *  - owning and
	 *  - non-owning.
	 *
	 * In the empty (default-constructed) state, there is no associated `FILE`
	 * handle and no I/O operations are possible.  (Attempting them is still
	 * well-defined but will result in an exception been `throw`n immediately.)
	 * The object will also enter the empty state when `close` is called or
	 * after moving away from it.
	 *
	 * The owning state is established by successfully constructing a
	 * `file_output` object from a file-name.  The constructor will open the
	 * file and the destructor will close it again.  It can also be closed
	 * explicitly by calling `close`.
	 *
	 * The non-owing state is reached by constructing a `file_output` object
	 * from an existing `FILE` pointer.  In this state, all I/O operations
	 * (including calling `close` explicitly) are still possible but the
	 * destructor will not close the file handle itself.  This is obviously a
	 * bad idea for file handles that must be closed so this constructor should
	 * only be used when constructing `file_output` objects from `stdout` which
	 * should never be closed by the program.
	 *
	 */
	class file_output final
	{
	public:

		/**
		 * @brief
		 *     Constructs a `file_output` object in the empty state.
		 *
		 * The object is not connected to any file handle and all I/O
		 * operations will fail by `throw`ing a `std::system_error` with error
		 * code `EBADF`.
		 *
		 */
		file_output() noexcept;

		/**
		 * @brief
		 *     Constructs a `file_output` object by opening the specified file.
		 *
		 * The file handle will be closed in the destructor, unless `close` is
		 * called explicitly before.
		 *
		 * The file will always be opened in `"wb"` mode.
		 *
		 * @param filename
		 *     operating-system specific path to the file to open
		 *
		 * @throws std::system_error
		 *     if the file cannot be opened
		 *
		 */
		explicit file_output(const std::string& filename);

		/**
		 * @brief
		 *     Constructs a `file_output` object from an existing file handle.
		 *
		 * The file handle will *not* be closed in the destructor.  `close` may
		 * still be called explicitly, though.
		 *
		 * The behavior is undefined unless `fp` refers to a valid file handle
		 * opened in `"wb"` mode.
		 *
		 * The `filename` parameter will only be used for error messages.  It
		 * doesn't have to refer to an existing file in the file-system and may
		 * be the empty string, indicating that no file-name is associated with
		 * the `FILE` pointer.
		 *
		 * @param fp
		 *     open `FILE` pointer to read the data from
		 *
		 * @param filename
		 *     informal file-name to use in error messages
		 *
		 */
		explicit file_output(std::FILE* fp, const std::string& filename = "");

		/**
		 * @brief
		 *     `delete`d copy constructor.
		 *
		 * `file_output` objects are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 */
		file_output(const file_output& other) = delete;

		/**
		 * @brief
		 *     `delete`d copy-assignment operator.
		 *
		 * `file_output` objects are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		file_output& operator=(const file_output& other) = delete;

		/**
		 * @brief
		 *     Move constructor.
		 *
		 * On completion, `other` is always left in the empty state and `*this`
		 * in the previous state of `other`.
		 *
		 * @param other
		 *     `file_output` object to transfer the state out of
		 *
		 */
		file_output(file_output&& other) noexcept;

		/**
		 * @brief
		 *     Move-assignment operator.
		 *
		 * This operation has the same effect as if first running the
		 * destructor and then the move constructor.
		 *
		 * Since there is no way to report errors from closing or flushing the
		 * previously managed file handle, this operation should be avoided on
		 * `file_output` objects that are not in the empty state.
		 *
		 * @param other
		 *     `file_output` object to transfer the state out of
		 *
		 * @returns
		 *     reference to `*this`
		 *
		 */
		file_output& operator=(file_output&& other) noexcept;

		/**
		 * @brief
		 *     Destroys the `file_output` object.
		 *
		 * Depending on the object's prior state this function will have the
		 * following effect.
		 *
		 *  - If the object was in the owning state, its file handle will be
		 *    closed.  If any error occurs during this operation, it is
		 *    suppressed.
		 *
		 *  - If the object was in the non-owning state, its file handle will
		 *    be flushed but not closed.  If any error occurs during this
		 *    operation, it is suppressed.
		 *
		 *  - Otherwise, if the object was in the empty state, this operation
		 *    has no effect.
		 *
		 * Since there is no way to report errors from closing or flushing the
		 * previously managed file handle, this operation should be avoided on
		 * `file_output` objects that are not in the empty state.
		 *
		 */
		~file_output() noexcept;

		/**
		 * @brief
		 *     Tests whether there is an open file handle associated with this
		 *     `file_output` object.
		 *
		 * Note that if this function `return`s `true`, this only means that
		 * there is an open file handle associated with the object.  It is
		 * impossible to test whether that handle can successfully be written
		 * to without actually going ahead and trying.  In other words, calling
		 * an I/O operation immediately after this function `return`ed `true`
		 * may still `throw`.
		 *
		 * @returns
		 *     `false` if the object is in the empty state, else `true`
		 *
		 */
		explicit operator bool() const noexcept;

		/**
		 * @brief
		 *     `return`s the file-name associated with this `file_output`
		 *     object.
		 *
		 * This is the same string that was passed as file-name to the
		 * constructor or an empty string, if the `file_output` object is in
		 * the empty state.
		 *
		 * @returns
		 *     file-name of the associated file
		 *
		 */
		const std::string& filename() const noexcept;

		/**
		 * @brief
		 *     Writes bytes to the associated file handle.
		 *
		 * If the `file_output` object is in the empty state, a
		 * `std::system_error` is `throw`n.  Otherwise, this function is a
		 * direct mapping to `std::fwrite` with automatic error checking.
		 *
		 * The file handle will not be flushed.  It is possible that the
		 * operation fails but this error will only manifest at a later point
		 * due to buffering.
		 *
		 * @param data
		 *     pointer to the first byte to write
		 *
		 * @param size
		 *     number of bytes to write
		 *
		 * @throws std::system_error
		 *     if the I/O operation fails
		 *
		 */
		void write(const void* data, std::size_t size);

		/**
		 * @brief
		 *     Writes the bytes from `text` to the associated file handle.
		 *
		 * This function will write exactly the bytes from `text`, including
		 * any embedded NUL bytes.  No character substitutions will be
		 * performed and no new-line character will be appended.
		 *
		 * This is a pure convenience function and has the exact same effect as
		 * calling `write(text.data(), text.size())`.  See the documentation of
		 * that overload for more details.
		 *
		 * @param text
		 *     bytes to write
		 *
		 * @throws std::system_error
		 *     if the I/O operation fails
		 *
		 */
		void write(const std::string& text);

		/**
		 * @brief
		 *     Prints formatted characters to the associated file handle.
		 *
		 * If the `file_output` object is in the empty state, a
		 * `std::system_error` is `throw`n.  Otherwise, this function is a
		 * direct mapping to `std::fprintf` with automatic error checking.
		 *
		 * The exact same format strings and parameters are accepted as by the
		 * underlying C-libraries `printf` function.  Including all undefined
		 * behavior that might be triggered when passing the wrong combination
		 * of arguments.
		 *
		 * No implicit new-line character will be appended.  If a terminating
		 * new-line character is desired, it has to be included in the format
		 * string explicitly.
		 *
		 * @param format
		 *     `printf`-style format string
		 *
		 * @param ...
		 *     parameters to be substituted into the format string
		 *
		 * @throws std::system_error
		 *     if the I/O operation fails
		 *
		 */
		[[gnu::format(__printf__, 2, 3)]]
		void print(const char* format, ...);

		/**
		 * @brief
		 *     Flushes the associated file handle.
		 *
		 * If the `file_output` object is in the empty state, a
		 * `std::system_error` is `throw`n.  Otherwise, this function is a
		 * direct mapping to `std::fflush` with automatic error checking.
		 *
		 * Prefer calling this function explicitly over relying on the
		 * destructor implicitly flushing `file_output` objects in the
		 * non-owning state as in the latter case, there is no way to report
		 * errors.
		 *
		 * @throws std::system_error
		 *     if the I/O operation fails
		 *
		 */
		void flush();

		/**
		 * @brief
		 *     Closes the associated file handle.
		 *
		 * If the `file_output` object is in the empty state, a
		 * `std::system_error` is `throw`n.  Otherwise, this function is a
		 * direct mapping to `std::fclose` with automatic error checking.  On
		 * completion, the `file_output` object will always be in the empty
		 * state.
		 *
		 * Prefer calling this function explicitly over relying on the
		 * destructor implicitly closing `file_output` objects in the owning
		 * state as in the latter case, there is no way to report errors.
		 *
		 * This function may also be called on `file_output` objects in the
		 * non-owning state, although desire to do so is probably a sign of
		 * questionable design in the calling code.
		 *
		 * @throws std::system_error
		 *     if the I/O operation fails
		 *
		 */
		void close();

		/**
		 * @brief
		 *     &ldquo;Finalizes&rdquo; the associated file handle.
		 *
		 * The action of this function depends on the `file_output` object's
		 * state.
		 *
		 *  - in the owning state, `close()` is called,
		 *  - in the non-owning state, `flush()` is called and
		 *  - in the empty state, nothing happens.
		 *
		 * That is, this function does exactly what the destructor will do plus
		 * error reporting.
		 *
		 * Prefer calling this function explicitly over relying on the
		 * destructor implicitly closing `file_output` objects in the owning
		 * state as in the latter case, there is no way to report errors.
		 *
		 * @throws std::system_error
		 *     if the I/O operation fails
		 *
		 */
		void finalize();

		/**
		 * @brief
		 *     `return`s the associated file handle.
		 *
		 * If the `file_output` object is in the empty state, the `nullptr`
		 * will be `return`ed.
		 *
		 * This function allows for low-level operations for which there is no
		 * equivalent in the API of this `class` to be performed directly on
		 * the file-handle.  This is cumbersome but allowed as long as the file
		 * handle is not invalidated.  In particular, calling `std::fclose` on
		 * the `return`ed `FILE` pointer will result in undefined behavior.
		 *
		 * @returns
		 *     associated native file handle
		 *
		 */
		std::FILE* handle() noexcept;

		/**
		 * @brief
		 *     Exchanges the associated file handles and file-names between two
		 *     `file_output` objects.
		 *
		 * @param lhs
		 *     first `file_output` object to have its state exchanged
		 *
		 * @param rhs
		 *     second `file_output` object to have its state exchanged
		 *
		 */
		friend void swap(file_output& lhs, file_output& rhs) noexcept;

	private:

		/** @brief Associated native file handle. */
		std::unique_ptr<std::FILE, void(*)(FILE*)> _handle;  // cannot haz '{}'

		/** @brief Associated informal file name. */
		std::string _filename{};

	};  // class file_output

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_IO_FILE_OUTPUT_HPP
#include "io/file_output.tpp"
#undef MINIJAVA_INCLUDED_FROM_IO_FILE_OUTPUT_HPP
