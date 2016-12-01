#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void* mj_runtime_allocate(const size_t sz, const size_t n)
{
	size_t nbytes;
	if (__builtin_mul_overflow(n, sz, &nbytes)) {
		fprintf(stderr, "minijava: allocate: memory request for %zu * %zu bytes too large\n", n, sz);
		abort();
	}
	void* memory = malloc(nbytes);
	if (memory == NULL) {
		fprintf(stderr, "minijava: allocate: %s\n", strerror(errno));
		abort();
	}
	return memory;
}
