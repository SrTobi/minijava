#include "runtime/runtime.hpp"

static const char source_code[] = R"C(
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void minijava_main(void);

static const char* program_name;

void* mj_runtime_allocate(const size_t sz, const size_t n)
{
	size_t nbytes;
	if (__builtin_mul_overflow(n, sz, &nbytes)) {
		fprintf(stderr, "%s: allocate: Memory request for %zu * %zu bytes too large\n", program_name, n, sz);
		abort();
	}
	void* memory = malloc(nbytes);
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
