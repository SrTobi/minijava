#ifndef MINIJAVA_INCLUDED_FROM_IO_FILE_DATA_HPP
#error "Never `#include <io/file_data.tpp>` directly, `#include <io/file_data.hpp>` instead."
#endif

#include <utility>


namespace minijava
{

	inline file_data::file_data(file_data&& other) noexcept : file_data{}
	{
		swap(*this, other);
	}

	inline file_data& file_data::operator=(file_data&& other) noexcept
	{
		file_data temp{};
		swap(*this, temp);
		swap(*this, other);
		return *this;
	}

	inline file_data::iterator file_data::begin() const noexcept
	{
		return static_cast<const char*>(data());
	}

	inline file_data::iterator file_data::end() const noexcept
	{
		return begin() + size();
	}

	inline const std::string& file_data::filename() const noexcept
	{
		return _filename;
	}

	inline void swap(file_data& lhs, file_data& rhs) noexcept
	{
		using std::swap;
		swap(lhs._pimpl, rhs._pimpl);
		swap(lhs._filename, rhs._filename);
	}

}  // namespace minijava
