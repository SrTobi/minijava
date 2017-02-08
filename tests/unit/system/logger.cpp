#include "system/logger.hpp"

#define BOOST_TEST_MODULE  system_logger
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE(empty_logger_does_nothing_on_printf)
{
	minijava::logger logger{};
	logger.printf("%s %i", "test", 123);
}

BOOST_AUTO_TEST_CASE(logger_printf_writes_to_file)
{
	std::string log;
	{
		char* buffer;
		size_t size;
		{
			FILE* stream = open_memstream (&buffer, &size);
			minijava::logger logger(stream, [](auto f) {fclose(f);});
			logger.printf("%s,%i,%.2f", "test", 123, 0.0f);
		}

		log = std::string{buffer, size};
		free(buffer);
	}

	BOOST_REQUIRE_EQUAL(log, "test,123,0.00");
}
