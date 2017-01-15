#pragma once

#include <memory>
#include <functional>

namespace minijava
{
	/**
	 * A simple logger
	 *
	 * Use logger::printf to write to the log. Log messages are written to a stream.
	 * An empty logger can be created to disable logging output.
	 */
	class logger
	{
	public:
		using close_t = std::function<void(FILE*)>;
	public:
		/**
		 * Creates an empty logger.
		 *
		 * Writing to such a logger does nothing.
		 */
		logger();

		/**
		 * Creates a logger to an io stream.
		 *
		 * @param dest the io file this logger writes to.
		 */
		logger(FILE* dest);

		/**
		 * Creates a logger to an io stream.
		 *
		 * @param dest the io file this logger writes to.
		 * @param closef a function called when the logger is detached from the stream.
		 */
		logger(FILE* dest, const close_t& closef);

		/**
		 * Writes a formated log message.
		 *
		 * @param fmt the format of the log message
		 * @param ... the data to fill the format with
		 */
		void printf(const char* fmt, ...) __attribute__((format (printf, 2, 3)));

	private:
		/** the io stream all log messages are written to */
		std::unique_ptr<FILE, close_t> _log_dest;
	};
}
