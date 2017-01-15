#include "logger.hpp"

#include <cstdarg>

namespace minijava
{
	logger::logger()
	{
	}

	logger::logger(FILE* dest)
		: _log_dest(dest, [](FILE*){})
	{
	}

	logger::logger(FILE* dest, const close_t& closef)
		: _log_dest(dest, closef)
	{
	}

	void logger::printf(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		FILE* dest = _log_dest.get();
		if(dest)
		{
			vfprintf(dest, fmt, args);
		}
		va_end(args);
	}
}
