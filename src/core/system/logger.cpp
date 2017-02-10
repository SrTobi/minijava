#include "logger.hpp"

#include <cstdarg>

namespace minijava
{
	logger::logger()
	{
	}

	logger::logger(std::FILE* dest)
		: _log_dest(dest, [](FILE*){})
	{
	}

	logger::logger(std::FILE* dest, const close_t& closef)
		: _log_dest(dest, closef)
	{
	}

	void logger::printf(const char* fmt, ...)
	{
		if(auto dest = _log_dest.get())
		{
			std::va_list args;
			va_start(args, fmt);
			std::vfprintf(dest, fmt, args);
			va_end(args);
		}
	}
}
