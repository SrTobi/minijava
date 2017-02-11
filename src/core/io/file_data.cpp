#include "io/file_data.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>
#include <system_error>
#include <vector>

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "exceptions.hpp"
#include "global.hpp"
#include "io/misc.hpp"


namespace minijava
{

	namespace /* anonymous */
	{

		struct file_data_impl
		{
			virtual const void * data() const noexcept = 0;
			virtual std::size_t size() const noexcept = 0;
			virtual bool good() const noexcept = 0;
			virtual void dispose() noexcept = 0;
			virtual ~file_data_impl() = default;
		};

		struct file_data_impl_none final : file_data_impl
		{
			const void * data() const noexcept override { return ""; }
			std::size_t size() const noexcept override { return 0; }
			bool good() const noexcept override { return false; }
			void dispose() noexcept override { /* do nothing */ }

			static file_data_impl_none * get_instance() noexcept
			{
				static file_data_impl_none instance{};
				return &instance;
			}
		};

		struct file_data_impl_fp final : file_data_impl
		{
			const void * data() const noexcept override { return vec.data(); }
			std::size_t size() const noexcept override { return vec.size(); }
			bool good() const noexcept override { return true; }
			void dispose() noexcept override { delete this; }

			static file_data_impl_fp * get_instance(std::FILE *const fp, const std::string& filename)
			{
				assert(fp != nullptr);
				auto buffer = std::vector<char>{};
				while (true) {
					const auto chunksize = std::size_t{4096};
					const auto offset = buffer.size();
					buffer.resize(offset + chunksize);
					const auto count = std::fread(buffer.data() + offset, 1, chunksize, fp);
					buffer.resize(offset + count);
					if (count > 0) {
						continue;
					} else if (std::feof(fp)) {
						break;
					} else if (std::ferror(fp)) {
						const auto ec = std::error_code{errno, std::system_category()};
						throw std::system_error{ec, "Cannot read file: " + filename};
					} else {
						MINIJAVA_NOT_REACHED();
					}
				}
				auto self = std::make_unique<file_data_impl_fp>();
				self->vec.swap(buffer);
				return self.release();
			}

			std::vector<char> vec{};
		};

		struct file_data_impl_mmap final : file_data_impl
		{
			const void * data() const noexcept override { return mr.get_address(); }
			std::size_t size() const noexcept override { return mr.get_size(); }
			bool good() const noexcept override { return true; }
			void dispose() noexcept override { delete this; }

			static file_data_impl_mmap * get_instance(const std::string& filename)
			{
				namespace ipc = boost::interprocess;
				auto self = std::make_unique<file_data_impl_mmap>();
				self->fm = ipc::file_mapping{filename.c_str(), ipc::read_only};
				self->mr = ipc::mapped_region{self->fm, ipc::read_only};
				return self.release();
			}

			boost::interprocess::file_mapping fm{};
			boost::interprocess::mapped_region mr{};
		};

		// Tells whether mmap() should be given a chance.  This is the default
		// unless the user sets the environment variable `MINIJAVA_NO_MMAP` to
		// a non-empty string.  This interface could be improved.  OTOH, this
		// functionality is mainly intended for benchmarking.
		bool wanna_try_mmap() {
			static const auto answer = [](){
				const auto envval = std::getenv(MINIJAVA_ENVVAR_NO_MMAP);
				return (envval == nullptr) || (std::strlen(envval) == 0);
			}();
			return answer;
		}

	}  // namespace /* anonymous */


	file_data::file_data() noexcept
		: _pimpl{file_data_impl_none::get_instance()}
	{
	}

	file_data::file_data(const std::string& filename)
		: _pimpl{nullptr}
		, _filename{filename}
	{
		check_file_name(filename);
		if (wanna_try_mmap()) {
			try {
				_pimpl = file_data_impl_mmap::get_instance(filename);
				return;
			} catch (const boost::interprocess::interprocess_exception& /* e */) {
				// Cannot mmap() file?  Let's try traditional I/O.
			}
		}
		auto fp = std::unique_ptr<std::FILE, decltype(&std::fclose)>{
			std::fopen(filename.c_str(), "rb"), &std::fclose
		};
		if (!fp) {
			const auto ec = std::error_code{errno, std::system_category()};
			throw std::system_error{ec, "Cannot open file: " + filename};
		}
		_pimpl = file_data_impl_fp::get_instance(fp.get(), filename);
	}

	file_data::file_data(std::FILE *const fp, const std::string& filename)
		: _pimpl{nullptr}
		, _filename{filename}
	{
		_pimpl = file_data_impl_fp::get_instance(fp, filename);
	}

	file_data::~file_data() noexcept
	{
		static_cast<file_data_impl*>(_pimpl)->dispose();
	}

	void file_data::dispose()
	{
		static_cast<file_data_impl*>(_pimpl)->dispose();
		_pimpl = file_data_impl_none::get_instance();
	}

	const void * file_data::data() const noexcept
	{
		return static_cast<file_data_impl*>(_pimpl)->data();
	}

	std::size_t file_data::size() const noexcept
	{
		return static_cast<file_data_impl*>(_pimpl)->size();
	}

	file_data::operator bool() const noexcept
	{
		return static_cast<file_data_impl*>(_pimpl)->good();
	}

}  // namespace minijava
