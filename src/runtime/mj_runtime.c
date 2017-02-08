#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>


__attribute__ ((sysv_abi))
extern void minijava_main(void);


static const char* program_name;
static jmp_buf exception_jump_buffer;


static inline int32_t maximum(const int32_t a, const int32_t b)
{
	return (a > b) ? a : b;
}

__attribute__ ((sysv_abi))
void* mj_runtime_new(const int32_t nmemb, const int32_t size)
{
	if (nmemb < 0) {
		fprintf(stderr, "%s: new: Request for negative array size %ld\n", program_name, (long) nmemb);
		longjmp(exception_jump_buffer, 1);
	}
	if (size <= 0) {
		fprintf(stderr, "%s: new: Request for non-positive object size %ld\n", program_name, (long) size);
		longjmp(exception_jump_buffer, 1);
	}
	/* Always allocate at least one byte to make sure arrays have unique addresses. */
	void* memory = calloc((size_t) maximum(1, nmemb), (size_t) size);
	if (memory == NULL) {
		fprintf(stderr, "%s: new: %s\n", program_name, strerror(errno));
		longjmp(exception_jump_buffer, 1);
	}
	return memory;
}

__attribute__ ((sysv_abi))
int32_t mj_runtime_id(const int32_t x)
{
	return x;
}

__attribute__ ((sysv_abi))
void mj_runtime_exit(const int32_t status)
{
	exit((int) status);
}

__attribute__ ((sysv_abi))
void mj_runtime_println(const int32_t n)
{
	const long promoted = n;  /* long is guaranteed to be at least 32 bit */
	if (printf("%ld\n", promoted) < 0) {
		fprintf(stderr, "%s: println: %s\n", program_name, strerror(errno));
		longjmp(exception_jump_buffer, 1);
	}
}

__attribute__ ((sysv_abi))
void mj_runtime_write(const int32_t b)
{
	/*
	  Cast the argument back and forth because an int may only provide 16 bits
	  of precision and signed overflow is undefined behavior in ISO C.
	*/
	const int octet = (int) (((unsigned) b) & 0xffU);
	if (fputc(octet, stdout) < 0) {
		fprintf(stderr, "%s: write: %s\n", program_name, strerror(errno));
		longjmp(exception_jump_buffer, 1);
	}
}

__attribute__ ((sysv_abi))
void mj_runtime_flush(void)
{
	if (fflush(stdout) < 0) {
		fprintf(stderr, "%s: flush: %s\n", program_name, strerror(errno));
		longjmp(exception_jump_buffer, 1);
	}
}

__attribute__ ((sysv_abi))
int32_t mj_runtime_read(void)
{
	const int c = fgetc(stdin);
	if ((c < 0) && ferror(stdin)) {
		fprintf(stderr, "%s: read: %s\n", program_name, strerror(errno));
		longjmp(exception_jump_buffer, 1);
	}
	return (int32_t) c;
}

int main(int argc, char** argv)
{
	program_name = (argc > 0) ? argv[0] : "minijava";
	if (argc > 1) {
		fprintf(stderr, "%s: Too many arguments\n", program_name);
		return EXIT_FAILURE;
	}
	switch (setjmp(exception_jump_buffer)) {
	case 0:
		minijava_main();
		return EXIT_SUCCESS;
	case 1:
		return EXIT_FAILURE;
	default:
		fprintf(stderr, "%s: main: %s\n", program_name, "Assertion failed");
		return EXIT_FAILURE;
	}
}
