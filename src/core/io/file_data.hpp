/**
 * @file file_data.hpp
 *
 * @brief
 *     A simple fast RAII wrapper around memory mapped files with graceful
 *     degration to traditional file I/O.
 *
 */

#pragma once

#include <cstdio>
#include <string>


namespace minijava
{

	/**
	 * @brief
	 *     A RAII wrapper around memory mapped files.
	 *
	 * Instances of this `class` represent the contents of a file.  The
	 * contents will always be loaded into memory by the constructor and then
	 * be available as a read-only region of memory exposed via the iterator
	 * interface.  When possible, the operating system's facilities to
	 * memory-map files will be used.  If this is not possible (for example,
	 * because the file is not a regular file), the strategy falls back to
	 * traditional I/O.  It is also possible to construct an object from open
	 * file handle which is important if you want to treat standard input just
	 * as other files.
	 *
	 * This `class` uses exceptions for error handling.  If any I/O operation
	 * fails, a `std::system_error` with a helpful error message (that will
	 * include the file-name if possible) will be `throw`n.  All operations not
	 * marked as `noexcept` might also `throw` `std::bad_alloc` exceptions if
	 * memory exhaustion occurs during their operation.
	 *
	 * The documentation of this `class`es member functions uses the terms
	 * *associate* and *disassociate* when talking about file data.  If file
	 * data is no longer associated with any `file_data` object or the
	 * `file_data` object it is associated with gets destroyed, the resources
	 * are always cleaned up properly.
	 *
	 * Note that while accessing the memory presented through the iterators is
	 * extremely fast, the operations on this `class` except for `filename` and
	 * `swap` have to go through some indirection (basically a `virtual`
	 * function call in a different translation unit) so calling `begin()` or
	 * `end()` ina tight loop is probably a bad idea.  It is better to obtain
	 * the iterators once an then use them as local variables.
	 *
	 */
	class file_data final
	{
	public:

		/**
		 * @brief
		 *     Contiguous iterator type used to expose the file data.
		 *
		 * Do not rely on this to be a raw pointer.  Use `data()` if you
		 * actually need a pointer rather than an iterator.
		 *
		 */
		using iterator = const char*;

		/**
		 * @brief
		 *     Creates a `file_data` object with no associated data.
		 *
		 */
		file_data() noexcept;

		/**
		 * @brief
		 *     Creates a `file_data` object with the contents of the named
		 *     file.
		 *
		 * @param filename
		 *     operating-system specific path to the file to read
		 *
		 * @throws std::system_error
		 *     if the file cannot be brought into memory
		 *
		 */
		explicit file_data(const std::string& filename);

		/**
		 * @brief
		 *     Creates a `file_data` object with data read from the provided
		 *     `FILE` pointer.
		 *
		 * The `FILE` pointer will not be closed after the data has been read.
		 * This must be done by the caller, if desired.  If `fp` is not a valid
		 * `FILE` pointer, the behavior is undefined.
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
		 * @throws std::system_error
		 *     if the file cannot be brought into memory
		 *
		 */
		explicit file_data(std::FILE * fp, const std::string& filename = "");

		/**
		 * @brief
		 *     `delete`d copy constructor.
		 *
		 * `file_data` objects are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 */
		file_data(const file_data& other) = delete;

		/**
		 * @brief
		 *     `delete`d copy-assignment operator.
		 *
		 * `file_data` objects are not copyable.
		 *
		 * @param other
		 *     *N/A*
		 *
		 * @returns
		 *     *N/A*
		 *
		 */
		file_data& operator=(const file_data& other) = delete;

		/**
		 * @brief
		 *     Move constructor.
		 *
		 * Creates a `file_data` object with the data previously associated
		 * with `other`.  On completion, `other` is always left in a
		 * default-constructed state and `*this` in the previous state of
		 * `other`.
		 *
		 * @param other
		 *     `file_data` object to transfer the associated data from
		 *
		 */
		file_data(file_data&& other) noexcept;

		/**
		 * @brief
		 *     Move-assignment operator.
		 *
		 * Disassociates any data from this `file_data` object, then associates
		 * the data previously associated with `other` with it.  On completion,
		 * `other` is always left in a default-constructed state and `*this` in
		 * the previous state of `other`.
		 *
		 * @param other
		 *     `file_data` object to transfer the associated data from
		 *
		 * @returns
		 *     reference to `*this`
		 *
		 */
		file_data& operator=(file_data&& other) noexcept;

		/**
		 * @brief
		 *     Destroys the `file_data` object.
		 *
		 */
		~file_data() noexcept;

		/**
		 * @brief
		 *     Disassociates any data from this `file_data` object and leaves
		 *     it in a default-constructed state.
		 *
		 * If there is no file data associated with this object to begin with,
		 * then this operation has no effect.
		 *
		 * This function is not marked `noexcept` because in theory, operating
		 * system functions to unmap memory could fail.  In practice, this
		 * function doesn't `throw` exceptions, though.
		 *
		 */
		void dispose();

		/**
		 * @brief
		 *     `return`s an iterator to the first byte of the data associated
		 *     with this `file_data` object.
		 *
		 * If there is no associated data, then a special past-the-end iterator
		 * is `return`ed which is indistinguishable from the iterator
		 * `returned` by a `file_data` object that is associated with data of
		 * zero size.  Use the contextual conversion to `bool` to find out
		 * whether the `file_data` object has any data associated with it if
		 * this difference matters.
		 *
		 * @returns
		 *     iterator to the first byte
		 *
		 */
		iterator begin() const noexcept;

		/**
		 * @brief
		 *     `return`s an iterator after the last byte of the data associated
		 *     with this `file_data` object.
		 *
		 * If there is no associated data, then the same iterator that will
		 * also be `return`ed by `begin()` will be `return`ed.  These iterators
		 * are indistinguishable from the iterators `returned` by a `file_data`
		 * object that is associated with data of zero size.  Use the
		 * contextual conversion to `bool` to find out whether the `file_data`
		 * object has any data associated with it if this difference matters.
		 *
		 * @returns
		 *     iterator after the last byte
		 *
		 */
		iterator end() const noexcept;

		/**
		 * @brief
		 *     `return`s a pointer to the first byte of the data associated
		 *     with this `file_data` object.
		 *
		 * The `return`ed pointer will never be the `nullptr` even if the size
		 * of the associated data is zero or there is no associated data at
		 * all.  The pointer must still not be dereferenced in these cases
		 * though.  However, it may safely be passed to standard library
		 * functions like `memcpy` that make arbitrary restrictions on the
		 * pointers that may be passed.
		 *
		 * @returns
		 *     pointer to the first byte
		 *
		 */
		const void * data() const noexcept;

		/**
		 * @brief
		 *     `return`s the size of the data associated with this `file_data`
		 *     object.
		 *
		 * If there is no data associated with this `file_data` object, zero
		 * will be `returned`.  This case is indistinguisheble from a
		 * `file_data` object that is associated with data of zero size.  Use
		 * the contextual conversion to `bool` to find out whether the
		 * `file_data` object has any data associated with it if this
		 * difference matters.
		 *
		 * @returns
		 *     size of the associated data in bytes
		 *
		 */
		std::size_t size() const noexcept;

		/**
		 * @brief
		 *     `return`s the file-name associated with this `file_data` object.
		 *
		 * This might be the empty string if there is no file or its name is
		 * not known or if the `file_data` object has no associated file data.
		 *
		 * @returns
		 *     file-name of the associated file
		 *
		 */
		const std::string& filename() const noexcept;

		/**
		 * @brief
		 *     Tests whether there is data associated with this `file_data`
		 *     object.
		 *
		 * This function will `return` `true` if there is associated data but
		 * its size is zero.
		 *
		 * @returns
		 *     whether there is data associated with this `file_data` object
		 *
		 */
		explicit operator bool() const noexcept;

		/**
		 * @brief
		 *     Associates the data previously associated with `lhs` with `rhs`
		 *     and vice versa.
		 *
		 * @param lhs
		 *     first `file_data` object to have its data re-associated
		 *
		 * @param rhs
		 *     second `file_data` object to have its data re-associated
		 *
		 */
		friend void swap(file_data& lhs, file_data& rhs) noexcept;

	private:

		/** @brief Pointer to internal state and implementation. */
		void * _pimpl{};

		/** @brief Associated file-name. */
		std::string _filename{};

	};  // class file_data

}  // namespace minijava


#define MINIJAVA_INCLUDED_FROM_IO_FILE_DATA_HPP
#include "io/file_data.tpp"
#undef MINIJAVA_INCLUDED_FROM_IO_FILE_DATA_HPP
