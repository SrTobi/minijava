#ifndef MINIJAVA_INCLUDED_FROM_IO_FILE_OUTPUT_HPP
#error "Never `#include <io/file_output.tpp>` directly; `#include <io/file_output.hpp>` instead."
#endif

#include <cstdarg>
#include <utility>

#include "exceptions.hpp"
#include "io/misc.hpp"


namespace minijava
{

	namespace detail
	{

		// `throw`s a `std::system_errror` with error code `EBADF` mentioning
		// `func` as the member function that was called.  The behavior is
		// udnefined if `func` is not a NUL terminated character array.
		[[noreturn]]
		void file_output_throw_empty_state(const char* func);

		// `throw`s a `std::system_errror` with the error code reported by the
		// global `errno` variable and `message` as the operation that failed
		// on file `filename`.  Both strings may be empty.
		[[noreturn]]
		void file_output_throw_errno(const std::string& message, const std::string& filename);

		// Deleters for our managed FILE pointer.  We have to compare their
		// addresses so we cannot use lambdas.

		inline void do_nothing_fp(std::FILE* /* fp */) noexcept
		{
		}

		inline void do_flush_fp(std::FILE* fp) noexcept
		{
			std::fflush(fp);
		}

		inline void do_close_fp(std::FILE* fp) noexcept
		{
			std::fclose(fp);
		}

	}  // namespace detail


	inline file_output::file_output() noexcept
		: _handle{nullptr, &detail::do_nothing_fp}
		, _filename{}
	{
	}

	inline file_output::file_output(std::FILE *const fp, const std::string& filename)
		: _handle{nullptr, &detail::do_flush_fp}
		, _filename{filename}
	{
		_handle.reset(fp);
	}

	inline file_output::file_output(const std::string& filename)
		: _handle{nullptr, detail::do_close_fp}
		, _filename{filename}
	{
		check_file_name(filename);
		_handle.reset(std::fopen(filename.c_str(), "wb"));
		if (!_handle) {
			detail::file_output_throw_errno("Cannot open file", _filename);
		}
	}

	inline file_output::file_output(file_output&& other) noexcept
		: file_output{}
	{
		swap(*this, other);
	}

	inline file_output& file_output::operator=(file_output&& other) noexcept
	{
		file_output temp{};
		swap(*this, temp);
		swap(*this, other);
		return *this;
	}

	inline file_output::~file_output() noexcept
	{
		// Nothing to do.  (But declared explicitly for exposition.)
	}

	inline file_output::operator bool() const noexcept
	{
		return bool(_handle);
	}

	inline const std::string& file_output::filename() const noexcept
	{
		return _filename;
	}

	inline void file_output::write(const void *const data, const std::size_t size)
	{
		if (!_handle) {
			detail::file_output_throw_empty_state("write");
		}
		auto start = static_cast<const char*>(data);
		auto remaining = size;
		while (remaining > 0) {
			const auto count = std::fwrite(start, 1, remaining, _handle.get());
			if ((count != remaining) && std::ferror(_handle.get())) {
				detail::file_output_throw_errno("Cannot write to file", _filename);
			}
			remaining -= count;
			start += count;
		}
	}

	inline void file_output::write(const std::string& text)
	{
		write(text.data(), text.size());
	}

	inline void file_output::print(const char* format, ...)
	{
		if (!_handle) {
			detail::file_output_throw_empty_state("print");
		}
		va_list vargs;
		va_start(vargs, format);
		const auto status = std::vfprintf(_handle.get(), format, vargs);
		va_end(vargs);
		if (status < 0) {
			detail::file_output_throw_errno("Cannot write to file", _filename);
		}
	}

	inline void file_output::flush()
	{
		if (!_handle) {
			detail::file_output_throw_empty_state("flush");
		}
		if (std::fflush(_handle.get()) != 0) {
			detail::file_output_throw_errno("Cannot flush output to file", _filename);
		}
	}

	inline void file_output::close()
	{
		if (!_handle) {
			detail::file_output_throw_empty_state("close");
		}
		const auto fnm = std::move(_filename);
		_filename.clear();
		if (std::fclose(_handle.release()) != 0) {
			detail::file_output_throw_errno("Cannot close file", fnm);
		}
	}

	inline void file_output::finalize()
	{
		if (_handle) {
			const auto delptr = _handle.get_deleter();
			if (delptr == &detail::do_nothing_fp) {
				// do nothing
			} else if (delptr == &detail::do_flush_fp) {
				flush();
			} else if (delptr == &detail::do_close_fp) {
				close();
			} else {
				MINIJAVA_NOT_REACHED();
			}
		}
	}

	inline std::FILE* file_output::handle() noexcept
	{
		return _handle.get();
	}

	inline void swap(file_output& lhs, file_output& rhs) noexcept
	{
		using std::swap;
		swap(lhs._handle, rhs._handle);
		swap(lhs._filename, rhs._filename);
	}

}  // namespace minijava
