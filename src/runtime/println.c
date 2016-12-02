#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void mj_runtime_println(const int32_t n)
{
	const long promoted = n;  /* long is guaranteed to be at least 32 bit */
	if (printf("%ld\n", promoted) < 0) {
		fprintf(stderr, "minijava: println: %s\n", strerror(errno));
		abort();
	}
}
