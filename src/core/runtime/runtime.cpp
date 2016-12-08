#include "runtime/runtime.hpp"

static const char source_code[] = R"C(
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void minijava_main(void);

static const char* program_name;

void* mj_runtime_allocate(const int32_t nmemb, const int32_t size)
{
	if (nmemb < 0) {
		fprintf(stderr, "%s: allocate: Request for negative array size %ld\n", program_name, (long) nmemb);
		abort();
	}
	if (size <= 0) {
		fprintf(stderr, "%s: allocate: Request for non-positive object size %ld\n", program_name, (long) size);
		abort();
	}
	const size_t nbytes = ((size_t) nmemb) * ((size_t) size);
    /* Always allocate at least one byte to make sure arrays have unique addresses. */
	void* memory = malloc(nbytes > 0 ? nbytes : 1);
	if (memory == NULL) {
		fprintf(stderr, "%s: allocate: %s\n", program_name, strerror(errno));
		abort();
	}
	return memory;
}

void mj_runtime_println(const int32_t n)
{
	const long promoted = n;  /* long is guaranteed to be at least 32 bit */
	if (printf("%ld\n", promoted) < 0) {
		fprintf(stderr, "%s: println: %s\n", program_name, strerror(errno));
		abort();
	}
}

int main(int argc, char** argv)
{
	program_name = (argc > 0) ? argv[0] : "minijava";
	if (argc > 1) {
		fprintf(stderr, "%s: Too many arguments\n", program_name);
		return EXIT_FAILURE;
	}
	minijava_main();
	return EXIT_SUCCESS;
}
)C";


namespace minijava
{

	std::string runtime_source()
	{
		return source_code;
	}

}  // namespace minijava
